package dev.nfs3hp.port;

import java.util.ArrayList;

/** Where each on-screen control sits by default.  Plain arithmetic in layout
 *  units -- the overlay's own coordinates, close to dp -- with nothing of
 *  Android in it, so every size, edge distance and screen shape can be checked
 *  off the device.
 *
 *  Everything stands on the grid the layout editor draws and snaps to, GRID
 *  units a cell.  Rows and columns are whole cells apart, and once mirroring
 *  and the distance from the edges have been applied each group of controls
 *  moves onto the grid as one, so a group keeps its own spacing exactly.  The
 *  small buttons share one size, 58 x 48 at 100%, and follow the size setting
 *  like everything else; the D-pad, the steering buttons, the pedals and the
 *  menu layout's confirm and back keep shapes of their own. */
final class TouchLayout
{
    static final float GRID = 8;

    /** One control's default box: its centre and its size. */
    static final class Box
    {
        final String action;
        final int group;
        float x, y;
        final float width, height;

        Box(String action, int group, float x, float y, float width, float height)
        {
            this.action = action;
            this.group = group;
            this.x = x;
            this.y = y;
            this.width = width;
            this.height = height;
        }

        float left() { return x - width / 2; }
        float top() { return y - height / 2; }
        float right() { return x + width / 2; }
        float bottom() { return y + height / 2; }
    }

    /* Groups that move onto the grid together, sideways.  Heights are placed on
     * it as they are worked out. */
    private static final int TOP_LEFT = 0, TOP_RIGHT = 1, STEERING = 2, ROWS = 3, PEDALS = 4, MENU_KEYS = 5;
    private static final int GROUPS = 6;

    private TouchLayout() {}

    /** The nearest grid line. */
    static float snap(float value)
    {
        return Math.round(value / GRID) * GRID;
    }

    /** A distance, rounded up to whole cells. */
    static float cells(float value)
    {
        return (float) Math.ceil(value / GRID - 1e-3) * GRID;
    }

    /**
     * @param width    the overlay's area, in layout units
     * @param height   likewise
     * @param edge     distance from the side edges, from the touch settings
     * @param raise    how far the lower controls are raised, likewise
     * @param size     the size setting, as a factor
     * @param physical one physical pixel, in layout units
     * @param separate menus and races have layouts of their own
     * @param menu     the menu layout, when they do
     * @param mirrored steering on the right
     */
    static ArrayList<Box> defaults(float width, float height, float edge, float raise, float size,
                                   float physical, boolean separate, boolean menu, boolean mirrored)
    {
        final float w = width - 2 * edge, h = height - raise;
        final float buttonWidth = 58 * size, buttonHeight = 48 * size;
        final float column = cells(buttonWidth + 6), row = cells(buttonHeight + 8);
        final boolean menuLayout = separate && menu;
        final ArrayList<Box> boxes = new ArrayList<>();

        /* The top rows: pause and its neighbours on the left; lights and
         * recovery on the right, or the keyboard in the menu layout. */
        final float topY = snap(18 + buttonHeight / 2);
        final float topBottom = topY + buttonHeight / 2;
        final String[] topLeft = separate ? new String[] { "pause", "mode" }
                                          : new String[] { "pause", "keyboard", "confirm", "back" };
        for (int i = 0; i < topLeft.length; ++i)
            boxes.add(new Box(topLeft[i], TOP_LEFT, 20 + buttonWidth / 2 + i * column, topY,
                              buttonWidth, buttonHeight));
        final float rightX = w - 20 - buttonWidth / 2;
        if (menuLayout)
            boxes.add(new Box("keyboard", TOP_RIGHT, rightX, topY, buttonWidth, buttonHeight));
        else
        {
            boxes.add(new Box("headlights", TOP_RIGHT, rightX - column, topY, buttonWidth, buttonHeight));
            boxes.add(new Box("recover", TOP_RIGHT, rightX, topY, buttonWidth, buttonHeight));
        }

        final float steerY, steerHalf;
        if (separate && !menu)
        {
            final float steerWidth = 68 * size, steerHeight = 76 * size;
            steerY = snap(h - 20 - steerHeight / 2);
            steerHalf = steerHeight / 2;
            final float leftX = 20 + steerWidth / 2;
            boxes.add(new Box("steer_left", STEERING, leftX, steerY, steerWidth, steerHeight));
            boxes.add(new Box("steer_right", STEERING, leftX + cells(steerWidth + 8 + 26 * physical), steerY,
                              steerWidth, steerHeight));
        }
        else
        {
            /* The shared layout stacks two rows above its D-pad, and on a short
             * screen or at a large size the D-pad gives up size rather than run
             * those rows into the top one. */
            float pad = 140 * size, y = snap(h - 20 - pad / 2);
            if (!separate)
                while (pad > 60 * size
                       && y - cells(pad / 2 + 8 + buttonHeight / 2) - row - buttonHeight / 2 < topBottom + 8)
                {
                    pad -= 2;
                    y = snap(h - 20 - pad / 2);
                }
            steerY = y;
            steerHalf = pad / 2;
            boxes.add(new Box("steering", STEERING, 20 + pad / 2, y, pad, pad));
        }

        if (menuLayout)
        {
            final float confirmWidth = 80 * size, confirmHeight = 84 * size;
            final float backWidth = 72 * size, backHeight = 64 * size;
            final float confirmX = w - 20 - confirmWidth / 2, confirmY = snap(h - 20 - confirmHeight / 2);
            boxes.add(new Box("confirm", MENU_KEYS, confirmX, confirmY, confirmWidth, confirmHeight));
            // Bottoms level with the confirm button's.
            boxes.add(new Box("back", MENU_KEYS, confirmX - confirmWidth / 2 - 16 * size - backWidth / 2,
                              confirmY + confirmHeight / 2 - backHeight / 2, backWidth, backHeight));
            return placed(boxes, w, edge, mirrored);
        }

        /* Horn and spikes right above the steering, the gears above them in the
         * same two columns.  Only a screen too short for that puts the gears
         * beside them instead. */
        final float hornY = steerY - cells(steerHalf + 8 + buttonHeight / 2);
        final float gearY = hornY - row;
        final float rowX = 20 + buttonWidth / 2;
        final boolean gearsAbove = gearY - buttonHeight / 2 >= topBottom + 4;
        boxes.add(new Box("horn", ROWS, rowX, hornY, buttonWidth, buttonHeight));
        boxes.add(new Box("spike_strip", ROWS, rowX + column, hornY, buttonWidth, buttonHeight));
        boxes.add(new Box("gear_down", ROWS, gearsAbove ? rowX : rowX + 2 * column, gearsAbove ? gearY : hornY,
                          buttonWidth, buttonHeight));
        boxes.add(new Box("gear_up", ROWS, gearsAbove ? rowX + column : rowX + 3 * column, gearsAbove ? gearY : hornY,
                          buttonWidth, buttonHeight));

        /* Brake is the outer pedal and throttle the inner one.  The gas pedal is
         * eight physical pixels wider than the brake and the gap between them
         * nineteen pixels more than twelve units, as tuned on the device; the
         * handbrake sits over the brake. */
        final float brakeWidth = 62 * size, brakeHeight = 84 * size;
        final float gasWidth = brakeWidth + 8 * physical, gasHeight = 118 * size;
        final float brakeX = w - 20 - 5 * physical - brakeWidth / 2;
        final float brakeY = snap(h - 20 - brakeHeight / 2);
        boxes.add(new Box("brake", PEDALS, brakeX, brakeY, brakeWidth, brakeHeight));
        boxes.add(new Box("accelerate", PEDALS, brakeX - brakeWidth / 2 - (12 + 19 * physical) - gasWidth / 2,
                          brakeY + brakeHeight / 2 - gasHeight / 2, gasWidth, gasHeight));
        boxes.add(new Box("handbrake", PEDALS, brakeX, brakeY - cells(brakeHeight / 2 + 8 + buttonHeight / 2),
                          buttonWidth, buttonHeight));

        /* Look behind and camera stand under recovery and lights, on the first of
         * these rows that keeps clear of the pedals: the gears' row, where the
         * two have always been, when it can.  A screen too short for any of them
         * gets the pair beside the handbrake instead, a column in from it. */
        final float[] rows = gearsAbove ? new float[] { gearY, hornY, gearY - row, topY + row }
                                        : new float[] { hornY, hornY - row, topY + row };
        float lookX = rightX, lookY = rows[rows.length - 1];
        int lookGroup = TOP_RIGHT;
        search:
        for (int group : new int[] { TOP_RIGHT, PEDALS })
        {
            final float x = group == TOP_RIGHT ? rightX : brakeX - column;
            for (float y : rows)
            {
                if (clear(boxes, group, x, y, buttonWidth, buttonHeight)
                    && clear(boxes, group, x - column, y, buttonWidth, buttonHeight))
                {
                    lookX = x;
                    lookY = y;
                    lookGroup = group;
                    break search;
                }
            }
        }
        boxes.add(new Box("look_behind", lookGroup, lookX, lookY, buttonWidth, buttonHeight));
        boxes.add(new Box("camera", lookGroup, lookX - column, lookY, buttonWidth, buttonHeight));
        return placed(boxes, w, edge, mirrored);
    }

    /* Whether a box of a group keeps half a cell clear of every box placed so
     * far -- and a whole cell sideways of other groups, since each group can
     * still move by up to half a cell as it goes onto the grid. */
    private static boolean clear(ArrayList<Box> boxes, int group, float x, float y, float width, float height)
    {
        for (Box b : boxes)
            if (Math.abs(x - b.x) < (width + b.width) / 2 + (b.group == group ? 1 : GRID)
                && Math.abs(y - b.y) < (height + b.height) / 2 + GRID / 2)
                return false;
        return true;
    }

    /* Mirrored for steering on the right, moved in from the edges, and each
     * group put on the grid by the offset that puts its first control there. */
    private static ArrayList<Box> placed(ArrayList<Box> boxes, float w, float edge, boolean mirrored)
    {
        final float[] shift = new float[GROUPS];
        final boolean[] known = new boolean[GROUPS];
        for (Box b : boxes)
        {
            if (mirrored)
                b.x = w - b.x;
            b.x += edge;
            if (!known[b.group])
            {
                known[b.group] = true;
                shift[b.group] = snap(b.x) - b.x;
            }
            b.x += shift[b.group];
        }
        return boxes;
    }
}
