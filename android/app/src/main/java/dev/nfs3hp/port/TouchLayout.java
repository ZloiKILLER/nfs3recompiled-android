package dev.nfs3hp.port;

import java.util.ArrayList;

/** Where each on-screen control sits by default.  Plain arithmetic in layout
 *  units -- the overlay's own coordinates, close to dp -- with nothing of
 *  Android in it, so every size, edge distance and screen shape can be checked
 *  off the device.
 *
 *  There are two layouts and the game says which one is up: one to drive with,
 *  and one for its menus, where a finger works the screen itself and only the
 *  two things a screen cannot offer are left -- back, and the keyboard.
 *
 *  Everything stands on the grid the layout editor draws and snaps to, GRID
 *  units a cell.  Rows and columns are whole cells apart, and once mirroring
 *  and the distance from the edges have been applied each group of controls
 *  moves onto the grid as one, so a group keeps its own spacing exactly.  The
 *  small buttons share one size, 58 x 48 at 100%, and follow the size setting
 *  like everything else; the steering buttons and the pedals keep shapes of
 *  their own. */
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
    private static final int TOP_LEFT = 0, TOP_RIGHT = 1, STEERING = 2, ROWS = 3, PEDALS = 4;
    private static final int GROUPS = 5;

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
     * @param menu     the game is showing a menu rather than a race
     * @param mirrored steering on the right
     */
    static ArrayList<Box> defaults(float width, float height, float edge, float raise, float size,
                                   float physical, boolean menu, boolean mirrored)
    {
        final float w = width - 2 * edge, h = height - raise;
        final float buttonWidth = 58 * size, buttonHeight = 48 * size;
        final float column = cells(buttonWidth + 6), row = cells(buttonHeight + 8);
        final ArrayList<Box> boxes = new ArrayList<>();

        /* The top row: pause on the left of a race, and on the right the lights
         * and recovery a race needs, or the back and keyboard a menu does. */
        final float topY = snap(18 + buttonHeight / 2);
        final float topBottom = topY + buttonHeight / 2;
        final float rightX = w - 20 - buttonWidth / 2;
        if (menu)
        {
            /* The whole of the menu layout: every other key a menu needs is the
             * screen itself.  Back takes the corner and the keyboard stands
             * under it, both out of the way of the menu's own buttons, which
             * the game keeps along the bottom. */
            boxes.add(new Box("back", TOP_RIGHT, rightX, topY, buttonWidth, buttonHeight));
            boxes.add(new Box("keyboard", TOP_RIGHT, rightX, topY + row, buttonWidth, buttonHeight));
            return placed(boxes, w, edge, mirrored);
        }
        boxes.add(new Box("pause", TOP_LEFT, 20 + buttonWidth / 2, topY, buttonWidth, buttonHeight));
        boxes.add(new Box("headlights", TOP_RIGHT, rightX - column, topY, buttonWidth, buttonHeight));
        boxes.add(new Box("recover", TOP_RIGHT, rightX, topY, buttonWidth, buttonHeight));

        /* Steering: two buttons, one for each way.  They used to share the
         * layout with a D-pad whose other two arms walked through menus; the
         * menus take a finger where it points now, so there is nothing left for
         * a D-pad to do. */
        final float steerWidth = 68 * size, steerHeight = 76 * size;
        final float steerY = snap(h - 20 - steerHeight / 2);
        final float steerHalf = steerHeight / 2;
        final float leftX = 20 + steerWidth / 2;
        boxes.add(new Box("steer_left", STEERING, leftX, steerY, steerWidth, steerHeight));
        boxes.add(new Box("steer_right", STEERING, leftX + cells(steerWidth + 8 + 26 * physical), steerY,
                          steerWidth, steerHeight));

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
