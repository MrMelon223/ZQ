from pyexpat import model
from tkinter import Image
import imgui
import glfw
from imgui.integrations.glfw import GlfwRenderer
import numpy as np
import OpenGL.GL as gl
import sqlite3 as sql
from PIL import Image

model_paths = [];
model_names = [];

asset_names = [];
asset_scales = [];
asset_lod0s = [];
asset_lod1s = [];
asset_lod2s = [];
asset_lod3s = [];
asset_materials = [];

model_names = [];
model_paths = [];

material_names = [];
material_colors = [];
material_normals = [];
material_glosses = [];

selected_asset_idx = -1;
show_asset_window = False;

def search_assets(search : str):
    r = [];

    count = 0;
    for name in asset_names:
        if (name == search):
            r.append((name, count));
        count = count + 1;
    return r;

def search_models(search : str):
    r = [];

    count = 0;
    for name in model_names:
        if (name == search):
            r.append((name, count));
        count = count + 1;
    return r;

def get_model_idx(search : str):
    count = 0;
    for name in model_names:
        if (name == search):
            return count;
        count = count + 1;
    return 0;

def get_material_idx(search : str):
    count = 0;
    for name in material_names:
        if (name == search):
            return count;
        count = count + 1;
    return 0;

def main():

    global show_asset_window;
    if not glfw.init():
        return;

    dim_x = 0;
    dim_y = 0;

    f = open("config.txt", 'r');

    count = 0;
    for line in f:
        if (count == 0):
            dim_x = np.int32(line.strip());
        elif (count == 1):
            dim_y = np.int32(line.strip());
        count = count + 1;

    window = glfw.create_window(dim_x, dim_y, "ZQasset_maker", None, None);
    glfw.make_context_current(window);
    imgui.create_context();
    impl = GlfwRenderer(window);

    io = imgui.get_io();
    io.display_size = dim_x, dim_y;

    imgui.style_colors_dark();

    bebas_neue_reg = io.fonts.add_font_from_file_ttf("fonts/Bebas_Neue/BebasNeue-Regular.ttf", 24);
    bebas_neue_title = io.fonts.add_font_from_file_ttf("fonts/Bebas_Neue/BebasNeue-Regular.ttf", 32);
    courier_prime_bold24 = io.fonts.add_font_from_file_ttf("fonts/Courier_Prime/CourierPrime-Bold.ttf", 24);
    courier_prime_reg24 = io.fonts.add_font_from_file_ttf("fonts/Courier_Prime/CourierPrime-Regular.ttf", 24);
    impl.refresh_font_texture();

    ##icon = Image.open("icon.png").convert('RGBA');
    ##w, h = icon.size;
    ##idata = icon.tobytes();

    ##glfw.set_window_icon(window, 1, [(w, h, idata)]);

    def remove_asset(idx):
        if (idx > 0 and idx < len(asset_names)):
            asset_names.pop(idx);
            asset_lod0s.pop(idx);
            asset_lod1s.pop(idx);
            asset_lod2s.pop(idx);
            asset_lod3s.pop(idx);
            asset_materials.pop(idx);

    def edit_asset(idx):
        if (idx >= 0 and idx < len(asset_names)):
         with imgui.font(courier_prime_bold24):
                imgui.set_next_window_size(dim_x / 2, dim_y, condition=imgui.ALWAYS);
                imgui.set_next_window_position(dim_x / 2, 0, condition=imgui.ALWAYS);
                imgui.begin(asset_names[idx], closable=False, flags=imgui.WINDOW_NO_COLLAPSE);
                with imgui.font(courier_prime_reg24):
                    ##imgui.text(asset_names[idx]);
                    changed_name, name = imgui.input_text("Asset Name", asset_names[idx], 1024);
                    changed_scale, scale = imgui.slider_float("Scale", np.float32(asset_scales[idx]), min_value = 0.01, max_value = 5.0, flags=imgui.SLIDER_FLAGS_ALWAYS_CLAMP);
                with imgui.font(courier_prime_bold24):
                    imgui.text("Level-of-Detail (LOD):");
                with imgui.font(courier_prime_reg24):
                    ##changed_lod0, lod0 = imgui.input_text("lod0 (closest)", asset_lod0s[idx], 1024);
                    expanded_lod0, current_item = imgui.combo("lod0 (closest)", get_model_idx(asset_lod0s[idx]), model_names);
                    if expanded_lod0:
                        asset_lod0s[idx] = model_names[current_item];

                    ##changed_lod1, lod1 = imgui.input_text("lod1", asset_lod1s[idx], 1024);
                    expanded_lod1, current_item1 = imgui.combo("lod1", get_model_idx(asset_lod1s[idx]), model_names);
                    if expanded_lod1:
                        asset_lod1s[idx] = model_names[current_item1];

                    ##changed_lod2, lod2 = imgui.input_text("lod2", asset_lod2s[idx], 1024);
                    expanded_lod2, current_item2 = imgui.combo("lod2", get_model_idx(asset_lod2s[idx]), model_names);
                    if expanded_lod0:
                        asset_lod2s[idx] = model_names[current_item2];

                    ##changed_lod3, lod3 = imgui.input_text("lod3 (furthest)", asset_lod3s[idx], 1024);
                    expanded_lod3, current_item3 = imgui.combo("lod3 (furthest)", get_model_idx(asset_lod3s[idx]), model_names);
                    if expanded_lod3:
                        asset_lod3s[idx] = model_names[current_item3];
                with imgui.font(courier_prime_bold24):
                    imgui.text("Material:");
                with imgui.font(courier_prime_reg24):
                    ##changed_material, material = imgui.input_text("Material", asset_materials[idx], 1024);
                    expanded_material, current_item_m = imgui.combo("Material", get_material_idx(asset_materials[idx]), material_names);
                    if expanded_material:
                        asset_materials[idx] = material_names[current_item_m];

                    if changed_name:
                        asset_names[idx] = name;

                    if imgui.button("Save"):
                        save_all();
                    if imgui.button("Delete"):
                        print("Deleting asset...");
                        remove_asset(idx);
                        ##imgui.end();
                        ##return;
                        show_asset_window = False;
        
                imgui.end();

    def draw_asset_list():
        global selected_asset_idx;
        global show_asset_window;
        with imgui.font(courier_prime_bold24):
            imgui.set_next_window_size(dim_x / 2, dim_y - 32, condition=imgui.ALWAYS);
            imgui.set_next_window_position(0, 32, condition=imgui.ALWAYS);
            imgui.begin("Assets", closable=False, flags=imgui.WINDOW_NO_COLLAPSE);
            changed_search, search = imgui.input_text("Search", "", 512);
            if imgui.button("New Asset"):
                print("Creating asset...");
                asset_names.append("new_asset");
                asset_scales.append("1.0");
                asset_lod0s.append("");
                asset_lod1s.append("");
                asset_lod2s.append("");
                asset_lod3s.append("");
                asset_materials.append("");
                show_asset_window = True;
                edit_asset(len(asset_names) - 1);


            imgui.text("Asset List:");
            if changed_search:
                r = search_assets(search);
                for tup in r:
                    n, idx = tup;
                    opened, selected = imgui.selectable(n, selected_asset_idx == idx);

                    if opened:
                        selected_asset_idx = i;
                        show_asset_window = True;
            else:
                i = 0;
                for name in asset_names:
                    opened, selected = imgui.selectable(name, selected_asset_idx == i);

                    if opened:
                        selected_asset_idx = i;
                        show_asset_window = True;

                    i = i + 1;

            imgui.end();


    connection = sql.connect("../resources/ZQmaster.db");
    cursor = connection.cursor();

    def save_all():
        print("Saving assets...");
        cursor.execute("DELETE FROM assets;");

        for i in range(len(asset_names)):
            cursor.execute("INSERT INTO assets (name, scale, lod0, lod1, lod2, lod3, material) VALUES (?, ?, ?, ?, ?, ?, ?)", (asset_names[i], asset_scales[i], asset_lod0s[i], asset_lod1s[i], asset_lod2s[i], asset_lod3s[i], asset_materials[i]));
        connection.commit();

    cursor.execute("SELECT * FROM models;");
    all_models = cursor.fetchall();

    for mod in all_models:
        model_paths.append(mod[0]);
        model_names.append(mod[1]);

    cursor.execute("SELECT * FROM assets;");
    all_assets = cursor.fetchall();

    for asset in all_assets:
        asset_names.append(asset[0]);
        asset_scales.append(asset[1]);
        asset_lod0s.append(asset[2]);
        asset_lod1s.append(asset[3]);
        asset_lod2s.append(asset[4]);
        asset_lod3s.append(asset[5]);
        asset_materials.append(asset[6]);

    cursor.execute("SELECT * FROM models;");
    all_models = cursor.fetchall();

    for model in all_models:
        model_paths.append(model[0]);
        model_names.append(model[1]);

    cursor.execute("SELECT * FROM materials;");
    all_mats = cursor.fetchall();

    for mat in all_mats:
        material_names.append(mat[0]);
        material_colors.append(mat[1]);
        material_normals.append(mat[2]);
        material_glosses.append(mat[3]);


    while not glfw.window_should_close(window):
        glfw.poll_events();
        impl.process_inputs();

        gl.glClearColor(0.1, 0.1, 0.1, 1.0);
        gl.glClear(gl.GL_COLOR_BUFFER_BIT or gl.GL_DEPTH_BUFFER_BIT);

        imgui.new_frame();
        
        with imgui.font(courier_prime_bold24):
            if imgui.begin_main_menu_bar():
                    if imgui.begin_menu("File"):
                        clicked_new, _ = imgui.menu_item("New Asset");
                        if clicked_new:
                            print("Creating asset...");
                            asset_names.append("new_asset");
                            asset_scales.append("1.0");
                            asset_lod0s.append("");
                            asset_lod1s.append("");
                            asset_lod2s.append("");
                            asset_lod3s.append("");
                            asset_materials.append("");
                            show_asset_window = True;
                            selected_asset_idx = len(asset_names) - 1; ## edit_asset(len(asset_names) - 1);
                
                        clicked_save, _ = imgui.menu_item("Save");
                        if clicked_save:
                            save_all();
                
                        imgui.end_menu();
            imgui.end_main_menu_bar();

        draw_asset_list();

        if show_asset_window == True:
            edit_asset(selected_asset_idx);

        imgui.render();
        impl.render(imgui.get_draw_data());
        glfw.swap_buffers(window);
        dim_x, dim_y = io.display_size;
    ##impl.shutdown()
    glfw.terminate();
    connection.commit();
    connection.close();

if __name__ == "__main__":
    main();
