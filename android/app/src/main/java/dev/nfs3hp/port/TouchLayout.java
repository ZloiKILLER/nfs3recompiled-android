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
    private static final int TOP_RIGHT = 0, STEERING = 1, ROWS = 2, PEDALS = 3;
    private static final int GROUPS = 4;

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
     * @param size     the size setting, as a factor
     * @param physical one physical pixel, in layout units
     * @param menu     the game is showing a menu rather than a race
     * @param mirrored steering on the right
     */
    static ArrayList<Box> defaults(float width, float height, float edge, float size,
                                   float physical, boolean menu, boolean mirrored)
    {
        final float w = width - 2 * edge, h = height;
        final float buttonWidth = 58 * size, buttonHeight = 48 * size;
        final float column = cells(buttonWidth + 6), row = cells(buttonHeight + 8);
        final ArrayList<Box> boxes = new ArrayList<>();

        /* The top row: pause on the left of a race, or the back and keyboard a
         * menu needs.  In a race each side is one uninterrupted five-button
         * column, matching the layout settled on in the on-device editor. */
        final float topY = snap(18 + buttonHeight / 2);
        final float rightX = w - 10 - buttonWidth / 2;
        if (menu)
        {
            /* A menu has no buttons at all.  The screen itself is what a menu
             * is worked with; the system's own back, gesture or button, is the
             * game's Escape; and the keyboard comes up by itself while the game
             * waits for a name (nfs3hp_main.cpp, textEntryTick).  Those two
             * were the only things a menu could not offer for itself, and
             * neither is missing now -- so the menus are left as the game drew
             * them, with nothing of ours on top. */
            return boxes;
        }
        boxes.add(new Box("recover", TOP_RIGHT, rightX, topY, buttonWidth, buttonHeight));
        boxes.add(new Box("headlights", TOP_RIGHT, rightX, topY + row, buttonWidth, buttonHeight));
        boxes.add(new Box("look_behind", TOP_RIGHT, rightX, topY + 2 * row, buttonWidth, buttonHeight));
        boxes.add(new Box("camera", TOP_RIGHT, rightX, topY + 3 * row, buttonWidth, buttonHeight));
        boxes.add(new Box("handbrake", TOP_RIGHT, rightX, topY + 4 * row, buttonWidth, buttonHeight));

        /* Steering: two buttons, one for each way.  They used to share the
         * layout with a D-pad whose other two arms walked through menus; the
         * menus take a finger where it points now, so there is nothing left for
         * a D-pad to do. */
        final float steerWidth = 68 * size, steerHeight = 76 * size;
        final float steerY = snap(h - 20 - steerHeight / 2);
        /* On the normal phone aspect ratio the steering remains at the edge,
         * below the column.  A very short preview cannot fit both vertically;
         * there the steering pair moves one column inward instead of covering
         * a button. */
        final float stackBottom = topY + 4 * row + GRID + buttonHeight / 2;
        final boolean tightLeft = stackBottom + 4 > steerY - steerHeight / 2;
        final float leftX = 20 + steerWidth / 2 + (tightLeft ? column : 0);
        final float steeringStep = cells(steerWidth + 8 + 26 * physical) - (tightLeft ? 2 * GRID : 0);
        boxes.add(new Box("steer_left", STEERING, leftX, steerY, steerWidth, steerHeight));
        boxes.add(new Box("steer_right", STEERING, leftX + steeringStep, steerY,
                          steerWidth, steerHeight));

        /* The final left column: pause, spikes, horn, plus and minus. */
        final float rowX = buttonWidth / 2;
        boxes.add(new Box("pause", ROWS, rowX, topY, buttonWidth, buttonHeight));
        boxes.add(new Box("spike_strip", ROWS, rowX, topY + row, buttonWidth, buttonHeight));
        boxes.add(new Box("horn", ROWS, rowX, topY + 2 * row, buttonWidth, buttonHeight));
        boxes.add(new Box("gear_up", ROWS, rowX, topY + 3 * row + GRID, buttonWidth, buttonHeight));
        boxes.add(new Box("gear_down", ROWS, rowX, topY + 4 * row + GRID, buttonWidth, buttonHeight));

        /* Brake is the outer pedal and throttle the inner one.  The gas pedal is
         * eight physical pixels wider than the brake and the gap between them
         * nineteen pixels more than twelve units, as tuned on the device. */
        final float brakeWidth = 62 * size, brakeHeight = 84 * size;
        final float gasWidth = brakeWidth + 8 * physical, gasHeight = 118 * size;
        final float brakeY = snap(h - 20 - brakeHeight / 2);
        final float utilityBottom = topY + 4 * row + buttonHeight / 2;
        final boolean tightRight = utilityBottom + 4 > brakeY - brakeHeight / 2;
        final float brakeX = w - 20 - 5 * physical - brakeWidth / 2 - (tightRight ? column : 0);
        boxes.add(new Box("brake", PEDALS, brakeX, brakeY, brakeWidth, brakeHeight));
        final float gasX = brakeX - brakeWidth / 2 - (12 + 19 * physical) - gasWidth / 2;
        final float gasY = brakeY + brakeHeight / 2 - gasHeight / 2;
        boxes.add(new Box("accelerate", PEDALS, gasX, gasY, gasWidth, gasHeight));
        /* A short preview moves the pedals inward to keep the right utility
         * column intact.  On the phone they stay at the edge below it. */
        return placed(boxes, w, edge, mirrored);
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
