import numpy as np

def export_level():
    f = open("test_level.zqlbin", 'wb');

    name = b"test_level";
    description = b"Test your skillz to pay the billz.";

    f.write(np.uint64(len(name)));
    f.write(name);
    f.write(np.uint64(len(description)));
    f.write(description);

    positions = [(np.float64(0.0), np.float64(0.0), np.float64(3.0)),
                 (np.float64(0.0), np.float64(0.0), np.float64(10.25)),
                 (np.float64(0.0), np.float64(0.0), np.float64(5.0))];

    rotations = [(np.float32(90.0), np.float32(180.0), np.float32(0.0)),
                 (np.float32(90.0), np.float32(0.0), np.float32(0.0)),
                 (np.float32(45.0), np.float32(-45.0), np.float32(180.0))];

    assets = [b"test_asset",
              b"test_asset",
              b"monkey_test"];

    N = len(positions);
    f.write(np.uint64(N));

    for i in range(N):
        x, y, z = positions[i];
        f.write(x);
        f.write(y);
        f.write(z);

        rx, ry, rz = rotations[i];

        f.write(rx);
        f.write(ry);
        f.write(rz);

        f.write(np.uint64(len(assets[i])));
        f.write(assets[i]);

    light_positions = [(np.float64(0.0), np.float64(5.0), np.float64(0.0))];

    light_forwards = [(np.float32(0.0), np.float32(-1.0), np.float32(0.0))];

    light_fovs = [(np.float32(90.0), np.float32(90.0))];

    light_fall_offs = [np.float32(8.0)];
    light_ranges = [np.float32(32.0)];
    light_roundnesses = [np.float32(0.0)];
    light_intensities = [np.float32(8.0)];

    light_diff_colors = [(np.uint8(255), np.uint8(255), np.uint8(255), np.uint8(255))];
    light_spec_colors = [(np.uint8(255), np.uint8(255), np.uint8(255), np.uint8(255))];

    light_shadow_type = [np.uint8(0)];

    N = len(light_positions);
    f.write(np.uint64(N));

    for i in range(N):
        x, y, z = light_positions[i];
        f.write(x);
        f.write(y);
        f.write(z);

        xf, yf, zf = light_forwards[i];
        f.write(xf);
        f.write(yf);
        f.write(zf);

        fovx, fovy = light_fovs[i];
        f.write(fovx);
        f.write(fovy);

        foff = light_fall_offs[i];
        f.write(foff);

        rnge = light_ranges[i];
        f.write(rnge);

        rndness = light_roundnesses[i];
        f.write(rndness);

        intensity = light_intensities[i];
        f.write(intensity);

        r, g, b, a = light_diff_colors[i];
        f.write(r);
        f.write(g);
        f.write(b);
        f.write(a);

        r, g, b, a = light_spec_colors[i];
        f.write(r);
        f.write(g);
        f.write(b);
        f.write(a);

    f.close();
export_level();

