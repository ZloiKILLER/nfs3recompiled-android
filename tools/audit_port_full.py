"""Read-only source/binary audit; writes reports only, never regenerates game code."""
from pathlib import Path
import re, struct, hashlib, json, collections, sys, zipfile

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'diagnostics' / 'full-port-audit'
sys.path.insert(0,str(ROOT/'disasm'))
import ordlookup

class PE:
    def __init__(self, path):
        self.path = Path(path); self.data = self.path.read_bytes(); b = self.data
        p = struct.unpack_from('<I', b, 60)[0]; o = p + 24
        self.base = struct.unpack_from('<I', b, o+28)[0]
        self.entry = self.base + struct.unpack_from('<I', b, o+16)[0]
        self.sections = []
        t = o + struct.unpack_from('<H', b, p+20)[0]
        for i in range(struct.unpack_from('<H', b, p+6)[0]):
            x = t + 40*i
            vs, va, size, off = struct.unpack_from('<IIII', b, x+8)
            self.sections.append(dict(name=b[x:x+8].rstrip(b'\0').decode('latin1'), rva=va, vsize=vs, size=size if off else 0, declared_size=size, off=off, flags=struct.unpack_from('<I', b, x+36)[0]))
        self.imports = set()
        imp = struct.unpack_from('<I', b, o+104)[0]
        if imp:
            x = self.offset(imp)
            while True:
                oft, _, _, name, ft = struct.unpack_from('<IIIII', b, x)
                if not name: break
                dll = self.string(name).lower(); k = self.offset(oft or ft)
                while True:
                    v = struct.unpack_from('<I', b, k)[0]; k += 4
                    if not v: break
                    if v & 0x80000000:
                        resolved=ordlookup.ordLookup(dll.encode(),v & 65535)
                        name=resolved.decode() if resolved else '#'+str(v & 65535)
                    else: name=self.string(v+2)
                    self.imports.add((dll,name))
                x += 20
        self.dynamic = set(m.group(1).decode() for m in re.finditer(rb'_([A-Za-z]\w{2,60})@\d{1,3}\x00', b))
    def offset(self, rva):
        for s in self.sections:
            if s['rva'] <= rva < s['rva'] + s['size']: return s['off']+rva-s['rva']
        raise ValueError(hex(rva))
    def string(self, rva):
        off=self.offset(rva); return self.data[off:self.data.index(b'\0',off)].decode('latin1')
    def read(self, va, size):
        try: off=self.offset(va-self.base); return self.data[off:off+size]
        except ValueError: return b''

LEX = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
def masked(text, strings=True):
    return LEX.sub(lambda m: ''.join('\n' if c=='\n' else ' ' for c in m[0]) if strings or m[0].startswith('/') else m[0],text)

# Enumerate ordinary definitions, including inline methods; not a C++ AST.
FUNC = re.compile(r'^[ \t]*(?:[\w:<>,*&~]+[ \t]+)+([\w:~]+)\s*\([^;{}]*\)\s*(?:const\s*)?(?:noexcept\s*)?\{',re.M)
def functions(text):
    clean=masked(text)
    for m in FUNC.finditer(clean):
        if m[1] in ('if','switch','while','for','catch','Packed','IEEEf80') or m[1].startswith('m_'): continue
        start=m.end()-1; depth=1; end=start+1
        while end<len(clean) and depth:
            depth += (clean[end]=='{')-(clean[end]=='}'); end+=1
        yield m[1], text.count('\n',0,m.start())+1, start+1,end-1,text[start+1:end-1]

def link(path,line): return f'[{path.relative_to(ROOT).as_posix()}:{line}]({path.as_posix()}:{line})'
def cell(s): return s.replace('|','\\|').replace('\n',' ')[:450]

def main():
    OUT.mkdir(exist_ok=True,parents=True)
    old=PE(ROOT/'nfs3hp/nfs3.exe'); modern=PE(ROOT.parent/'sourcedata/nfs3.exe')
    dlls=[PE(p) for p in (ROOT/'nfs3hp').glob('*.dll')]
    old_names={n for pe in [old]+dlls for _,n in pe.imports}
    modern_names={n for _,n in modern.imports}
    rows=[]; markers=[]; ignored=[]; all_functions=[]
    files=sorted(set((ROOT/'src/lib').rglob('*.cpp'))|set((ROOT/'include').rglob('*.h')))
    for path in files:
        text=path.read_text(encoding='utf-8',errors='replace'); clean=masked(text,False)
        fs=list(functions(text))
        for name,line,start,end,body in fs:
            all_functions.append((path,name,line))
            b=masked(body,False)
            params=re.findall(r'NFS2_USE\((\w+)\)',b)
            rest_uses=re.sub(r'NFS2_USE\([^;]*\);','',b)
            unused=[p for p in params if p not in ('app','cpu') and not re.search(r'\b'+re.escape(p)+r'\b',rest_uses)]
            if unused: ignored.append((path,line,name,unused))
            constraints=re.findall(r'NFS2_ASSERT\((.*?)\);',b,re.S)
            reduced=re.sub(r'NFS2_(?:USE|ASSERT)\([^;]*\);','',b)
            reduced=re.sub(r'SDL_Log\w*\([^;]*\);','',reduced).strip()
            reduced=re.sub(r'NFS_MSG_TRACE\([^;]*\);','',reduced).strip()
            category=None
            if not reduced: category='NOOP / только проверки'
            elif re.fullmatch(r'return\s+(?:-?\d+|nullptr|TRUE|FALSE|S_OK|x86::reg32\(-?\d+\))\s*;',reduced): category='Константный ответ (кандидат)'
            if category and 'false' in constraints: category='Полная заглушка: assert(false)'
            if not category and any(c.strip()=='false' for c in constraints): category='Неподдержанная ветка / защитный assert'
            if not category and re.search(r'TODO|FIXME|unimplemented|not implemented',body,re.I): category='TODO / требует ручной оценки'
            if not category and constraints: category='Ограничения аргументов / инварианты'
            if not category: continue
            short=name.split('::')[-1]
            ev=[]
            if '::' not in name:
                if short in old_names: ev.append('импорт EXE/DLL базы')
                if short in old.dynamic: ev.append('строка динамического символа базы')
                if short in modern_names: ev.append('импорт Modern')
                if short in modern.dynamic: ev.append('строка Modern')
            detail='; '.join(constraints) if constraints else reduced or 'нет операций'
            rows.append(dict(file=path.relative_to(ROOT).as_posix(),line=line,name=name,category=category,evidence='; '.join(ev) or 'по имени не установлено; возможен vtable/внутренний вызов',detail=cell(detail)))
        for m in re.finditer(r'NFS2_ASSERT\(false\)|TODO|FIXME|unimplemented|not implemented',text,re.I):
            owner=next((f[0] for f in fs if f[2]<=m.start()<f[3]),'вне распознанной функции')
            markers.append(dict(file=path.relative_to(ROOT).as_posix(),line=text.count('\n',0,m.start())+1,owner=owner,marker=m[0],context=cell(text[max(0,m.start()-60):m.start()+150])))
    # Explicit silent/partial cases not reliably detected by syntax alone.
    manual={
      'grLfbLock':'READ_ONLY возвращает 0; указатель выдается только WRITE_ONLY',
      'grLfbUnlock':'READ_ONLY возвращает 0',
      'grDepthBufferMode':'WBUFFER/compare-to-bias получают тот же путь, что ZBUFFER',
      'GetWindowLongA':'игнорирует индекс/окно, возвращает 0; диагностические счетчики не являются реализацией',
      'SetWindowsHookExA':'callback не регистрируется, возвращается 0',
      'GetLastError':'всегда 0; сохранение последней Win32 ошибки отсутствует',
      'loadGlFunctions':'допустимый NOOP на GLES: функции линкуются напрямую, НЕ дефект',
      'GetMessageA':'HWND/фильтры не обеспечены нижележащим Window::getMessageImpl',
      'Window::getMessageImpl':'window/filterMin/msgMax игнорируются; SDL очередь общая',
      'Window::getMessage':'проверить маршрутизацию потока/окна в getMessageImpl',
      'grTexClampMode':'API не применяет отдельные режимы S/T; shader уже имеет собственный clamp/wrap',
      'grTexFilterMode':'режимы не передаются в renderer; shader уже делает четыре выборки',
      'GlideRenderer::setTextureData':'TODO mipmaps; ограниченная поддержка форматов',
    }
    for path,name,line in all_functions:
        if name not in manual: continue
        found=next((r for r in rows if r['file']==path.relative_to(ROOT).as_posix() and r['line']==line),None)
        if found: found['detail']+='; '+manual[name]
        else: rows.append(dict(file=path.relative_to(ROOT).as_posix(),line=line,name=name,category='Ручная проверка: частичная реализация',evidence='см. исходник; runtime не доказан',detail=manual[name]))
    for row in rows:
        if row['name']=='GetWindowLongA': row['category']='Константный ответ (кандидат)'
    rows.sort(key=lambda r:(r['category'],r['file'],r['line']))
    counts=collections.Counter(r['category'] for r in rows)
    report=['# Аудит реализации порта — 2026-09-06','',
      'Сканированы все собственные src/lib/**/*.cpp и include/**/*.h. Таблица охватывает полные заглушки, константные ответы, неподдержанные ветки и ограничения. Это статический аудит с ручными уточнениями, не доказательство корректности каждой функции и не полный семантический разбор ARM64. SDL/сторонние библиотеки не классифицируются как код порта.',
      '',f'Файлов: {len(files)}; распознано определений: {len(all_functions)}; строк таблицы: {len(rows)}.',
      '', 'Импорт/строка символа не доказывает вызов. Константа/пустая функция может быть допустимой адаптацией. Защитные assert и допустимые ограничения не следует считать потерянными функциями. Сканируются обе стороны #ifdef; они не обязательно включены в текущую сборку.',
      '', '| Категория | Количество |','|---|---:|']
    report += [f'| {k} | {v} |' for k,v in counts.items()]
    report += ['', '| Функция / место | Категория | Статические свидетельства | Что реализовано / ограничение |','|---|---|---|---|']
    report += [f"| `{r['name']}` {link(ROOT/r['file'],r['line'])} | {r['category']} | {r['evidence']} | `{r['detail']}` |" for r in rows]
    (OUT/'implementation-table.md').write_text('\n'.join(report)+'\n',encoding='utf-8')
    (OUT/'implementation-table.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2),encoding='utf-8')
    (OUT/'all-markers.json').write_text(json.dumps(markers,ensure_ascii=False,indent=2),encoding='utf-8')
    manifest={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
    (OUT/'source-hashes.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
    artifacts=[]
    for p in [ROOT/'build-win/nfs3hp.exe',ROOT/'android/app/build/outputs/apk/debug/app-debug.apk']:
        if not p.exists(): continue
        data=p.read_bytes()
        artifacts.append(dict(path=p.as_posix(),bytes=len(data),sha256=hashlib.sha256(data).hexdigest()))
        if p.suffix=='.apk':
            with zipfile.ZipFile(p) as z:
                for name in z.namelist():
                    if name.endswith('/libmain.so'):
                        elf=z.read(name)
                        artifacts.append(dict(path=name,bytes=len(elf),sha256=hashlib.sha256(elf).hexdigest(),elf_machine=struct.unpack_from('<H',elf,18)[0]))
    (OUT/'artifact-hashes.json').write_text(json.dumps(artifacts,indent=2),encoding='utf-8')
    ig=['# Явно игнорируемые параметры','',
        'Дополнительная проверка частичных реализаций: NFS2_USE(param), после удаления которого параметр больше не упоминается в теле. Это не автоматически дефект. app/cpu исключены. Аргументы, используемые только в логировании, этим проходом не выявляются.',
        '', '| Функция / место | Параметры |','|---|---|']
    ig += [f'| `{name}` {link(path,line)} | `{", ".join(params)}` |' for path,line,name,params in ignored]
    (OUT/'ignored-parameters.md').write_text('\n'.join(ig)+'\n',encoding='utf-8')
    # Sweep every emitted NFS3 EXE instruction comment against the original bytes.
    checked=0; unique=set(); mismatches=[]; traps=[]; modern_diff=0; modern_absent=0
    for path in sorted((ROOT/'src/nfs3hp/disassembly').glob('*.cpp')):
        text=path.read_text(errors='replace')
        if re.fullmatch(r'nfs3hp\.\d+\.cpp',path.name):
            for m in re.finditer(r'// ([0-9a-fA-F]{8})  ([0-9a-f]+)\s',text):
                va=int(m[1],16); data=bytes.fromhex(m[2]); checked+=1
                unique.add((va,m[2]))
                if old.read(va,len(data))!=data: mismatches.append(hex(va))
                md=modern.read(va,len(data))
                if not md: modern_absent+=1
                elif md!=data: modern_diff+=1
        last=''; prev=''; owner=''
        for line,s in enumerate(text.splitlines(),1):
            if 'void ' in s and 'sub_' in s: owner=s.strip()
            if re.search(r'// [0-9a-fA-F]{8} ',s): last=s.strip()
            if 'NFS2_ASSERT(false)' in s:
                kind='защита dispatcher/default'
                if prev.startswith('//'):
                    kind='исходный INT3' if re.search(r'[-+]int3\b',last) else 'инструкция заменена assert'
                traps.append(dict(file=path.name,line=line,instruction=last,kind=kind,owner=owner))
            if s.strip(): prev=s.strip()
    (OUT/'generated-traps.json').write_text(json.dumps(traps,indent=2),encoding='utf-8')
    tr=['# Неподдержанные инструкции в generated NFS3 и DLL','',
        'Прямые замены инструкции на assert(false) из disasm/module.py. Наличие в дереве не доказывает достижимость: возможны неиспользуемые CRT-пути и ошибочно распознанные данные. Повтор одного VA бывает из-за дублированных generated-обёрток. INT3 и default диспетчера сюда не включены.',
        '', '| Модуль / место | Исходная инструкция |','|---|---|']
    tr += [f"| {link(ROOT/'src/nfs3hp/disassembly'/r['file'],r['line'])} | `{r['instruction']}` |" for r in traps if r['kind']=='инструкция заменена assert']
    tr+=['','Ручное уточнение: DAS по VA 0x47bbc0 и AAS по VA 0x4ca48b окружены последовательностями, похожими на таблицы указателей/float-данные, декодированные как инструкции. Их нельзя считать реально исполняемыми пробелами без проверки достижимости. FNSAVE/FRSTOR/LES находятся также в CRT-подобных ветках; отсутствие реализации реально в C++, но использование игрой ещё не установлено.']
    (OUT/'unsupported-instructions.md').write_text('\n'.join(tr)+'\n',encoding='utf-8')
    (OUT/'generated-byte-check.json').write_text(json.dumps(dict(checked=checked,unique_instructions=len(unique),mismatches=mismatches,modern_different_at_same_va=modern_diff,modern_unmapped_at_same_va=modern_absent),indent=2),encoding='utf-8')
    br=['# Сравнение входного EXE и Modern Patch','',
      'Сравнение выполнено по файлам и PE-таблицам. Различие байтов по одному VA не означает потерю функции: код мог быть перемещён. Наличие строки не доказывает исполнение.', '', '| Файл | Размер | SHA256 | Entry VA |','|---|---:|---|---|']
    for pe in (old,modern): br.append(f'| {pe.path.as_posix()} | {len(pe.data)} | {hashlib.sha256(pe.data).hexdigest()} | {pe.entry:#x} |')
    br+=['','У старых PE VirtualSize=0. Для BSS PointerToRawData=0: это не байты файла; заявленный размер учитывается отдельно.','','| EXE | Секция | RVA | Virtual / declared raw / file-backed | SHA256 file-backed |','|---|---|---|---|---|']
    for label,pe in [('База',old),('Modern',modern)]:
        for s in pe.sections:
            raw=pe.data[s['off']:s['off']+s['size']]
            br.append(f"| {label} | {s['name']} | {s['rva']:#x} | {s['vsize']} / {s['declared_size']} / {s['size']} | {hashlib.sha256(raw).hexdigest() if raw else 'нет байтов файла'} |")
    br+=['','## Изменения секций по одинаковым RVA','','| Секция базы | Сравнено байтов | Отличается | Нет отображения Modern |','|---|---:|---:|---:|']
    for s in old.sections:
        raw=old.data[s['off']:s['off']+s['size']]; other=modern.read(old.base+s['rva'],len(raw))
        # Avoid comparing across section boundaries.
        match=next((x for x in modern.sections if x['rva']==s['rva']),None)
        common=min(len(raw),match['size']) if match else 0
        br.append(f"| {s['name']} | {common} | {sum(a!=b for a,b in zip(raw[:common],other[:common]))} | {len(raw)-common} |")
    br += ['',f'Сверено {checked} комментариев инструкций C++ ({len(unique)} уникальных адрес/байты) с байтами базового EXE; несовпадений {len(mismatches)}. Это проверка происхождения, НЕ проверка семантики C++. Из них {modern_diff} отличаются по тому же VA в Modern, {modern_absent} не отображены.',
      '', f'В generated NFS3+DLL найдено {len(traps)} assert(false): {dict(collections.Counter(t["kind"] for t in traps))}. Полный список — generated-traps.json. Исходный INT3 и default диспетчера не являются доказательством потерянной функции.',
      '', '## Новые импорты Modern относительно базового EXE','','| DLL | Имя | Есть регистрация символа в src/lib |','|---|---|---|']
    registrations='\n'.join(p.read_text(errors='replace') for p in (ROOT/'src/lib').rglob('*.cpp'))
    registered=set(re.findall(r'\.registerSymbol\("([^"]+)"',registrations))
    added=sorted(modern.imports-old.imports)
    for dll,name in added: br.append(f'| {dll} | {name} | {"да (семантика отдельно)" if name in registered else "не найдена"} |')
    br+=['','## Импорты, отсутствующие в Modern','','| DLL | Имя |','|---|---|']
    br += [f'| {dll} | {name} |' for dll,name in sorted(old.imports-modern.imports)]
    br+=['','## Имена списков меню','','| Имя | База: offset | Modern: offset |','|---|---:|---:|']
    for name in ['fog','mirrorlevel','rearcamera','hudtypesingle','hudtypesplit']:
        key=name.encode()+b'\0'; br.append(f'| {name} | {old.data.find(key)} | {modern.data.find(key)} |')
    br+=['','## Изменения, описанные в комплектном readme Modern','',
         'Локальный sourcedata/readme_en.txt идентифицирует комплект как v1.6.2 beta (2023/09/13). Это идентификация комплекта, не криптографическая аттестация EXE. Подробности и номера строк см. основной обзор.',
         '', '## Практический вывод','',
         'Modern — исполняемые изменения, а не только другой fedata. Секция кода сохраняет размер и RVA, но содержит десятки тысяч отличающихся байтов. Размер BSS и расположение поздних секций изменены. Генератор использует фиксированные адреса data_segments/merge/split/thread segments и размещает DLL после секций EXE; поэтому подстановка Modern требует отдельной адаптации разметки, новых API и проверки драйверов. Новый импорт не означает, что он нужен на каждом запуске, но его нужно поддержать или доказать недостижимость соответствующего пути.']
    (OUT/'binary-comparison.md').write_text('\n'.join(br)+'\n',encoding='utf-8')
    print(json.dumps(dict(files=len(files),functions=len(all_functions),categories=counts,generated_instructions=checked,byte_mismatches=len(mismatches),traps=len(traps),new_imports=len(added)),ensure_ascii=False))

if __name__=='__main__': main()
