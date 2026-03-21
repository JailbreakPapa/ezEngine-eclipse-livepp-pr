"""Material setup utilities for ezEngine hair export.

Generates valid .ezMaterialAsset files in ezEngine's DDL serialization format.
"""

import struct
import random
import os


def _make_uuid():
    """Generate a pair of u64 values representing an ezEngine UUID."""
    return (random.getrandbits(64), random.getrandbits(64))


def _format_uuid(uuid_pair):
    """Format a UUID pair as DDL: u4{low,high}"""
    return f"u4{{{uuid_pair[0]},{uuid_pair[1]}}}"


def _float_to_hex(value):
    """Convert a Python float to ezEngine DDL hex representation.

    ezEngine encodes IEEE 754 float32 as a little-endian hex u32.
    For example, 1.0 (IEEE 754: 0x3F800000) is stored as 0x0000803F.
    Zero is encoded as plain 0.
    """
    if value == 0.0:
        return "0"
    # Pack as big-endian float32, then reverse bytes for little-endian hex
    be_bytes = struct.pack('>f', value)
    le_bytes = be_bytes[::-1]
    hex_val = ''.join(f'{b:02X}' for b in le_bytes)
    return f"0x{hex_val}"


def _double_to_hex(value):
    """Convert a Python float to ezEngine DDL hex representation for doubles.

    64-bit IEEE 754 double, little-endian byte order.
    """
    if value == 0.0:
        return "0"
    be_bytes = struct.pack('>d', value)
    le_bytes = be_bytes[::-1]
    hex_val = ''.join(f'{b:02X}' for b in le_bytes)
    return f"0x{hex_val}"


def _color_ddl(r, g, b, a=1.0):
    """Format an RGBA color as DDL: Color %Name{f{rHex,gHex,bHex,aHex}}"""
    return f"f{{{_float_to_hex(r)},{_float_to_hex(g)},{_float_to_hex(b)},{_float_to_hex(a)}}}"


# Preset functions for common hair colors via melanin model

def melanin_for_color(color_name):
    """Return melanin and melaninRedness values for common hair colors.

    Args:
        color_name: One of 'BLACK', 'BROWN', 'BLONDE', 'RED', 'AUBURN', 'GRAY', 'WHITE', etc.

    Returns:
        Tuple of (melanin, melaninRedness).
    """
    presets = {
        "black": (0.95, 0.0),
        "dark_brown": (0.7, 0.0),
        "brown": (0.5, 0.0),
        "light_brown": (0.35, 0.05),
        "blonde": (0.15, 0.05),
        "platinum": (0.05, 0.0),
        "red": (0.4, 0.8),
        "auburn": (0.5, 0.5),
        "ginger": (0.25, 0.9),
        "gray": (0.1, 0.0),
        "white": (0.0, 0.0),
    }
    return presets.get(color_name.lower(), (0.5, 0.0))


def get_material_setup_info(color_name):
    """Return a human-readable description of how to set up a hair material in ezEngine.

    Args:
        color_name: Hair color preset name.

    Returns:
        String with setup instructions.
    """
    melanin, redness = melanin_for_color(color_name)
    return (
        f"Create a new Material in ezEngine editor with:\n"
        f"  Shader: Shaders/Materials/HairStrandMaterial.ezShader\n"
        f"  Melanin: {melanin}\n"
        f"  MelaninRedness: {redness}\n"
    )


def generate_material_asset(output_path, color_name, abc_filepath=None):
    """Generate a valid .ezMaterialAsset file for hair strand rendering.

    Creates a DDL file that the ezEngine editor can open and resave. The file
    references HairStrandMaterial.ezShader and configures melanin-based color
    parameters from the given preset.

    Args:
        output_path: Path to write the .ezMaterialAsset file.
        color_name: Hair color preset name (e.g. 'BROWN', 'BLACK', 'RED').
        abc_filepath: Optional path to the source .abc file (for reference tracking).

    Returns:
        True on success, False on failure.
    """
    shader_path = "Shaders/Materials/HairStrandMaterial.ezShader"
    dither_guid = "{ ac614d7c-2b31-4a7b-aa0c-c5d8200b7b89 }"

    melanin, melanin_redness = melanin_for_color(color_name)

    # Generate UUIDs for the three main objects
    doc_uuid = _make_uuid()
    shader_props_uuid = _make_uuid()
    mat_props_uuid = _make_uuid()
    # The document root always has the same UUID in ezEngine
    doc_root_uuid = (18096612296587978288, 6449934965513159559)

    # Generate UUIDs for Types section objects
    shader_type_uuid = _make_uuid()
    shader_type_base_uuid = _make_uuid()
    doc_root_type_uuid = _make_uuid()
    reflected_class_uuid = _make_uuid()
    mat_props_type_uuid = _make_uuid()
    mat_shader_mode_uuid = _make_uuid()
    enum_base_uuid = _make_uuid()

    # Property descriptor UUIDs for the shader properties
    prop_uuids = {}
    shader_properties = [
        ("Melanin", "float"),
        ("MelaninRedness", "float"),
        ("UseCustomColor", "bool"),
        ("CustomHairColor", "ezColor"),
        ("UseColorTexture", "bool"),
        ("ColorTexture", "ezString"),
        ("PrimaryRoughness", "float"),
        ("SecondaryRoughness", "float"),
        ("PrimaryShift", "float"),
        ("SecondaryShift", "float"),
        ("ScatterIntensity", "float"),
        ("MultipleScatterScale", "float"),
        ("TransmissionRoughness", "float"),
        ("RootDarkening", "float"),
        ("TipFade", "float"),
        ("MaskThreshold", "float"),
        ("DitherNoiseTexture", "ezString"),
        ("BLEND_MODE", "BLEND_MODE"),
        ("SHADING_MODE", "SHADING_MODE"),
    ]
    for name, _ in shader_properties:
        prop_uuids[name] = _make_uuid()

    # Build properties block for shader properties object
    shader_prop_lines = []
    shader_prop_lines.append(f'\t\ts %BLEND_MODE{{"BLEND_MODE::BLEND_MODE_OPAQUE"}}')
    shader_prop_lines.append(f'\t\tColor %CustomHairColor{{{_color_ddl(0.3, 0.15, 0.05, 1.0)}}}')
    shader_prop_lines.append(f'\t\ts %ColorTexture{{""}}')
    shader_prop_lines.append(f'\t\ts %DitherNoiseTexture{{"{dither_guid}"}}')
    shader_prop_lines.append(f'\t\tf %MaskThreshold{{{_float_to_hex(0.3)}}}')
    shader_prop_lines.append(f'\t\tf %Melanin{{{_float_to_hex(melanin)}}}')
    shader_prop_lines.append(f'\t\tf %MelaninRedness{{{_float_to_hex(melanin_redness)}}}')
    shader_prop_lines.append(f'\t\tf %MultipleScatterScale{{{_float_to_hex(1.0)}}}')
    shader_prop_lines.append(f'\t\tf %PrimaryRoughness{{{_float_to_hex(0.1)}}}')
    shader_prop_lines.append(f'\t\tf %PrimaryShift{{{_float_to_hex(-0.1)}}}')
    shader_prop_lines.append(f'\t\tf %RootDarkening{{{_float_to_hex(0.3)}}}')
    shader_prop_lines.append(f'\t\tf %ScatterIntensity{{{_float_to_hex(1.0)}}}')
    shader_prop_lines.append(f'\t\tf %SecondaryRoughness{{{_float_to_hex(0.3)}}}')
    shader_prop_lines.append(f'\t\tf %SecondaryShift{{{_float_to_hex(0.1)}}}')
    shader_prop_lines.append(f'\t\ts %SHADING_MODE{{"SHADING_MODE::SHADING_MODE_LIT"}}')
    shader_prop_lines.append(f'\t\tf %TipFade{{{_float_to_hex(0.0)}}}')
    shader_prop_lines.append(f'\t\tf %TransmissionRoughness{{{_float_to_hex(0.15)}}}')
    shader_prop_lines.append(f'\t\tb %UseColorTexture{{0}}')
    shader_prop_lines.append(f'\t\tb %UseCustomColor{{0}}')

    shader_props_block = '\n'.join(shader_prop_lines)

    # Build property UUIDs list for the Types section shader type descriptor
    prop_uuid_lines = '\n'.join(
        f'\t\t\tUuid{{{_format_uuid(prop_uuids[name])}}}' for name, _ in shader_properties
    )

    # Build property descriptors for Types section
    prop_descriptors = []
    for name, ptype in shader_properties:
        flags = "ezPropertyFlags::StandardType|ezPropertyFlags::Phantom"
        if ptype in ("BLEND_MODE", "SHADING_MODE"):
            flags = "ezPropertyFlags::IsEnum|ezPropertyFlags::Phantom"
        prop_descriptors.append(
            f'o\n{{\n'
            f'\tUuid %id{{{_format_uuid(prop_uuids[name])}}}\n'
            f'\ts %t{{"ezReflectedPropertyDescriptor"}}\n'
            f'\tu3 %v{{2}}\n'
            f'\tp\n'
            f'\t{{\n'
            f'\t\tVarArray %Attributes{{}}\n'
            f'\t\ts %Category{{"ezPropertyCategory::Member"}}\n'
            f'\t\tInvalid %ConstantValue{{}}\n'
            f'\t\ts %Flags{{"{flags}"}}\n'
            f'\t\ts %Name{{"{name}"}}\n'
            f'\t\ts %Type{{"{ptype}"}}\n'
            f'\t}}\n'
            f'}}'
        )

    # Compute a hash (just use a deterministic but unique value)
    hash_val = (doc_uuid[0] ^ doc_uuid[1]) & 0xFFFFFFFFFFFFFFFF

    # Build references array
    references = [shader_path, dither_guid]

    references_block = '\n'.join(f'\t\t\ts{{"{ref}"}}' for ref in references)
    deps_block = f'\t\t\ts{{"{shader_path}"}}'
    package_deps_block = f'\t\t\ts{{"{dither_guid}"}}'

    content = f"""HeaderV2
{{
o
{{
\tUuid %id{{{_format_uuid(doc_uuid)}}}
\ts %t{{"ezAssetDocumentInfo"}}
\tu3 %v{{2}}
\ts %n{{"Header"}}
\tp
\t{{
\t\ts %AssetType{{"Material"}}
\t\tVarArray %Dependencies
\t\t{{
{deps_block}
\t\t}}
\t\tUuid %DocumentID{{{_format_uuid(doc_uuid)}}}
\t\tu4 %Hash{{{hash_val}}}
\t\tVarArray %MetaInfo{{}}
\t\tVarArray %Outputs{{}}
\t\tVarArray %PackageDeps
\t\t{{
{package_deps_block}
\t\t}}
\t\tVarArray %References
\t\t{{
{references_block}
\t\t}}
\t\ts %Tags{{""}}
\t}}
}}
}}
Objects
{{
o
{{
\tUuid %id{{{_format_uuid(shader_props_uuid)}}}
\ts %t{{"{shader_path}"}}
\tu3 %v{{2}}
\tp
\t{{
{shader_props_block}
\t}}
}}
o
{{
\tUuid %id{{{_format_uuid(mat_props_uuid)}}}
\ts %t{{"ezMaterialAssetProperties"}}
\tu3 %v{{4}}
\tp
\t{{
\t\ts %AssetFilterTags{{""}}
\t\ts %BaseMaterial{{""}}
\t\ts %Shader{{"{shader_path}"}}
\t\ts %ShaderMode{{"ezMaterialShaderMode::File"}}
\t\tUuid %ShaderProperties{{{_format_uuid(shader_props_uuid)}}}
\t\ts %Surface{{""}}
\t}}
}}
o
{{
\tUuid %id{{{_format_uuid(doc_root_uuid)}}}
\ts %t{{"ezDocumentRoot"}}
\tu3 %v{{1}}
\ts %n{{"ObjectTree"}}
\tp
\t{{
\t\tVarArray %Children
\t\t{{
\t\t\tUuid{{{_format_uuid(mat_props_uuid)}}}
\t\t}}
\t}}
}}
}}
Types
{{
o
{{
\tUuid %id{{{_format_uuid(shader_type_uuid)}}}
\ts %t{{"ezReflectedTypeDescriptor"}}
\tu3 %v{{1}}
\tp
\t{{
\t\tVarArray %Attributes{{}}
\t\ts %Flags{{"ezTypeFlags::Class|ezTypeFlags::Phantom"}}
\t\tVarArray %Functions{{}}
\t\ts %ParentTypeName{{"ezShaderTypeBase"}}
\t\ts %PluginName{{"ShaderTypes"}}
\t\tVarArray %Properties
\t\t{{
{prop_uuid_lines}
\t\t}}
\t\ts %TypeName{{"{shader_path}"}}
\t\tu3 %TypeVersion{{2}}
\t}}
}}
o
{{
\tUuid %id{{{_format_uuid(shader_type_base_uuid)}}}
\ts %t{{"ezReflectedTypeDescriptor"}}
\tu3 %v{{1}}
\tp
\t{{
\t\tVarArray %Attributes{{}}
\t\ts %Flags{{"ezTypeFlags::Class|ezTypeFlags::Abstract|ezTypeFlags::Phantom"}}
\t\tVarArray %Functions{{}}
\t\ts %ParentTypeName{{"ezReflectedClass"}}
\t\ts %PluginName{{"ShaderTypes"}}
\t\tVarArray %Properties{{}}
\t\ts %TypeName{{"ezShaderTypeBase"}}
\t\tu3 %TypeVersion{{2}}
\t}}
}}
o
{{
\tUuid %id{{{_format_uuid(doc_root_type_uuid)}}}
\ts %t{{"ezReflectedTypeDescriptor"}}
\tu3 %v{{1}}
\tp
\t{{
\t\tVarArray %Attributes{{}}
\t\ts %Flags{{"ezTypeFlags::Class|ezTypeFlags::Minimal"}}
\t\tVarArray %Functions{{}}
\t\ts %ParentTypeName{{"ezReflectedClass"}}
\t\ts %PluginName{{"Static"}}
\t\tVarArray %Properties{{}}
\t\ts %TypeName{{"ezDocumentRoot"}}
\t\tu3 %TypeVersion{{1}}
\t}}
}}
o
{{
\tUuid %id{{{_format_uuid(reflected_class_uuid)}}}
\ts %t{{"ezReflectedTypeDescriptor"}}
\tu3 %v{{1}}
\tp
\t{{
\t\tVarArray %Attributes{{}}
\t\ts %Flags{{"ezTypeFlags::Class|ezTypeFlags::Minimal"}}
\t\tVarArray %Functions{{}}
\t\ts %ParentTypeName{{""}}
\t\ts %PluginName{{"Static"}}
\t\tVarArray %Properties{{}}
\t\ts %TypeName{{"ezReflectedClass"}}
\t\tu3 %TypeVersion{{1}}
\t}}
}}
o
{{
\tUuid %id{{{_format_uuid(mat_props_type_uuid)}}}
\ts %t{{"ezReflectedTypeDescriptor"}}
\tu3 %v{{1}}
\tp
\t{{
\t\tVarArray %Attributes{{}}
\t\ts %Flags{{"ezTypeFlags::Class|ezTypeFlags::Minimal"}}
\t\tVarArray %Functions{{}}
\t\ts %ParentTypeName{{"ezReflectedClass"}}
\t\ts %PluginName{{"ezEditorPluginAssets"}}
\t\tVarArray %Properties{{}}
\t\ts %TypeName{{"ezMaterialAssetProperties"}}
\t\tu3 %TypeVersion{{4}}
\t}}
}}
o
{{
\tUuid %id{{{_format_uuid(mat_shader_mode_uuid)}}}
\ts %t{{"ezReflectedTypeDescriptor"}}
\tu3 %v{{1}}
\tp
\t{{
\t\tVarArray %Attributes{{}}
\t\ts %Flags{{"ezTypeFlags::IsEnum|ezTypeFlags::Minimal"}}
\t\tVarArray %Functions{{}}
\t\ts %ParentTypeName{{"ezEnumBase"}}
\t\ts %PluginName{{"ezEditorPluginAssets"}}
\t\tVarArray %Properties{{}}
\t\ts %TypeName{{"ezMaterialShaderMode"}}
\t\tu3 %TypeVersion{{1}}
\t}}
}}
o
{{
\tUuid %id{{{_format_uuid(enum_base_uuid)}}}
\ts %t{{"ezReflectedTypeDescriptor"}}
\tu3 %v{{1}}
\tp
\t{{
\t\tVarArray %Attributes{{}}
\t\ts %Flags{{"ezTypeFlags::Class|ezTypeFlags::Minimal"}}
\t\tVarArray %Functions{{}}
\t\ts %ParentTypeName{{""}}
\t\ts %PluginName{{"Static"}}
\t\tVarArray %Properties{{}}
\t\ts %TypeName{{"ezEnumBase"}}
\t\tu3 %TypeVersion{{1}}
\t}}
}}
{chr(10).join(prop_descriptors)}
}}
"""

    try:
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    except Exception:
        return False
