"""Blender operators for ezEngine hair export."""

import bpy
import os
from bpy.props import StringProperty, FloatProperty, BoolProperty, IntProperty, EnumProperty
from bpy_extras.io_utils import ExportHelper

from .hair_export import export_hair_curves_alembic, get_hair_curve_objects
from .material_setup import melanin_for_color, get_material_setup_info, generate_material_asset


class EZHAIR_OT_export(bpy.types.Operator, ExportHelper):
    """Export hair curves to Alembic (.abc) for ezEngine"""
    bl_idname = "ezhair.export"
    bl_label = "Export Hair to ezEngine"
    bl_options = {'REGISTER', 'UNDO'}

    filename_ext = ".abc"

    filter_glob: StringProperty(
        default="*.abc",
        options={'HIDDEN'},
    )

    selected_only: BoolProperty(
        name="Selected Only",
        description="Only export selected hair curve objects",
        default=True,
    )

    global_scale: FloatProperty(
        name="Global Scale",
        description="Scale factor applied to all exported data",
        default=1.0,
        min=0.001,
        max=1000.0,
    )

    width_scale: FloatProperty(
        name="Width Scale",
        description="Multiplier for strand widths",
        default=1.0,
        min=0.01,
        max=100.0,
    )

    generate_material: BoolProperty(
        name="Generate Material",
        description="Create an .ezMaterialAsset file configured for HairStrandMaterial.ezShader",
        default=True,
    )

    hair_color_preset: EnumProperty(
        name="Hair Color",
        description="Preset hair color for the generated material",
        items=[
            ('BROWN', "Brown", "Medium brown hair"),
            ('BLACK', "Black", "Black hair"),
            ('DARK_BROWN', "Dark Brown", "Dark brown hair"),
            ('LIGHT_BROWN', "Light Brown", "Light brown hair"),
            ('BLONDE', "Blonde", "Blonde hair"),
            ('PLATINUM', "Platinum", "Platinum/white blonde hair"),
            ('RED', "Red", "Red hair"),
            ('AUBURN', "Auburn", "Auburn hair"),
            ('GINGER', "Ginger", "Ginger hair"),
            ('GRAY', "Gray", "Gray hair"),
            ('WHITE', "White", "White hair"),
        ],
        default='BROWN',
    )

    generate_asset: BoolProperty(
        name="Generate Asset File",
        description="(Not recommended) Create a placeholder ezHairStrandAsset file. "
                    "Prefer importing the .abc through the ezEngine editor instead",
        default=False,
    )

    def draw(self, context):
        layout = self.layout

        layout.label(text="Export Settings:")
        layout.prop(self, "selected_only")
        layout.prop(self, "global_scale")
        layout.prop(self, "width_scale")

        layout.separator()
        layout.label(text="ezEngine Integration:")
        layout.prop(self, "generate_material")
        if self.generate_material:
            layout.prop(self, "hair_color_preset")
        layout.prop(self, "generate_asset")

    def execute(self, context):
        settings = {
            'selected_only': self.selected_only,
            'global_scale': self.global_scale,
            'width_scale': self.width_scale,
        }

        result, message = export_hair_curves_alembic(context, self.filepath, settings)

        if result == {'CANCELLED'}:
            self.report({'ERROR'}, message)
            return {'CANCELLED'}

        self.report({'INFO'}, message)

        # Generate material asset file
        if self.generate_material:
            mat_path = os.path.splitext(self.filepath)[0] + '.ezMaterialAsset'
            if generate_material_asset(mat_path, self.hair_color_preset, self.filepath):
                melanin, redness = melanin_for_color(self.hair_color_preset)
                self.report({'INFO'},
                    f"Generated material asset: {os.path.basename(mat_path)} "
                    f"(Melanin={melanin}, MelaninRedness={redness})")
            else:
                self.report({'WARNING'},
                    "Failed to generate material asset file.")

        # Generate asset document
        if self.generate_asset:
            self.report({'WARNING'},
                "Hair strand asset generation is not yet supported from Blender. "
                "Import the .abc file through the ezEngine editor instead.")

        return {'FINISHED'}

    def invoke(self, context, event):
        # Check for hair curve objects
        hair_objects = get_hair_curve_objects(context, self.selected_only)
        if not hair_objects:
            self.report({'ERROR'}, "No hair curve objects found in selection.")
            return {'CANCELLED'}
        return ExportHelper.invoke(self, context, event)


class EZHAIR_OT_quick_export(bpy.types.Operator):
    """Quick export all selected hair curves to the default path"""
    bl_idname = "ezhair.quick_export"
    bl_label = "Quick Export Hair"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        prefs = context.preferences.addons[__package__].preferences
        export_dir = bpy.path.abspath(prefs.default_export_path)

        if not os.path.isdir(export_dir):
            self.report({'ERROR'}, f"Default export path does not exist: {export_dir}")
            return {'CANCELLED'}

        # Use the blend file name as the default export name
        blend_name = os.path.splitext(os.path.basename(bpy.data.filepath))[0]
        if not blend_name:
            blend_name = "untitled_hair"

        filepath = os.path.join(export_dir, blend_name + "_hair.abc")

        settings = {
            'selected_only': True,
            'global_scale': 1.0,
            'width_scale': 1.0,
        }

        result, message = export_hair_curves_alembic(context, filepath, settings)

        if result == {'CANCELLED'}:
            self.report({'ERROR'}, message)
            return {'CANCELLED'}

        self.report({'INFO'}, message)
        return {'FINISHED'}


# File menu export entry
def menu_func_export(self, context):
    self.layout.operator(EZHAIR_OT_export.bl_idname, text="ezEngine Hair (.abc)")


classes = (
    EZHAIR_OT_export,
    EZHAIR_OT_quick_export,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
