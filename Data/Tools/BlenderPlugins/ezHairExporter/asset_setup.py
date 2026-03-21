"""Hair strand asset document generation for ezEngine.

Generates valid .ezHairStrandAsset DDL files that reference .abc source files.
When opened in the ezEngine editor, these are auto-transformed into the runtime
.ezBinHairStrands format via the HairImporter library.
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
    """Convert a Python float to ezEngine DDL hex representation (IEEE 754 float32, LE)."""
    if value == 0.0:
        return "0"
    be_bytes = struct.pack('>f', value)
    le_bytes = be_bytes[::-1]
    hex_val = ''.join(f'{b:02X}' for b in le_bytes)
    return f"0x{hex_val}"


def generate_hair_strand_asset(output_path, abc_relative_path, settings=None):
    """Generate a valid .ezHairStrandAsset DDL file.

    Creates a document that the ezEngine editor can open, which references
    the source .abc file. The editor will auto-transform it into the runtime
    .ezBinHairStrands binary format.

    Args:
        output_path: Absolute path to write the .ezHairStrandAsset file.
        abc_relative_path: Data-directory-relative path to the .abc file.
        settings: Optional dict with import settings:
            - global_width_scale (float, default 1.0)
            - default_width (float, default 0.0005)
            - tip_width_fraction (float, default 0.1)
            - max_strands_per_group (int, default 0 = unlimited)
            - generate_uvs (bool, default True)

    Returns:
        True on success, False on failure.
    """
    if settings is None:
        settings = {}

    global_width_scale = settings.get('global_width_scale', 1.0)
    default_width = settings.get('default_width', 0.0005)
    tip_width_fraction = settings.get('tip_width_fraction', 0.1)
    max_strands = settings.get('max_strands_per_group', 0)
    generate_uvs = settings.get('generate_uvs', True)

    doc_uuid = _make_uuid()
    props_uuid = _make_uuid()
    # The document root always has this UUID in ezEngine
    doc_root_uuid = (18096612296587978288, 6449934965513159559)

    # Type descriptor UUIDs
    props_type_uuid = _make_uuid()
    doc_root_type_uuid = _make_uuid()
    reflected_class_uuid = _make_uuid()

    # Hash for change detection
    hash_val = (doc_uuid[0] ^ doc_uuid[1]) & 0xFFFFFFFFFFFFFFFF

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
\t\ts %AssetType{{"Hair Strands"}}
\t\tVarArray %Dependencies
\t\t{{
\t\t\ts{{"{abc_relative_path}"}}
\t\t}}
\t\tUuid %DocumentID{{{_format_uuid(doc_uuid)}}}
\t\tu4 %Hash{{{hash_val}}}
\t\tVarArray %MetaInfo{{}}
\t\tVarArray %Outputs{{}}
\t\tVarArray %PackageDeps{{}}
\t\tVarArray %References
\t\t{{
\t\t\ts{{"{abc_relative_path}"}}
\t\t}}
\t\ts %Tags{{""}}
\t}}
}}
}}
Objects
{{
o
{{
\tUuid %id{{{_format_uuid(props_uuid)}}}
\ts %t{{"ezHairStrandAssetProperties"}}
\tu3 %v{{1}}
\tp
\t{{
\t\tf %DefaultWidth{{{_float_to_hex(default_width)}}}
\t\ts %File{{"{abc_relative_path}"}}
\t\tb %GenerateUVs{{{1 if generate_uvs else 0}}}
\t\tf %GlobalWidthScale{{{_float_to_hex(global_width_scale)}}}
\t\tu3 %MaxStrandsPerGroup{{{max_strands}}}
\t\tf %TipWidthFraction{{{_float_to_hex(tip_width_fraction)}}}
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
\t\t\tUuid{{{_format_uuid(props_uuid)}}}
\t\t}}
\t}}
}}
}}
Types
{{
o
{{
\tUuid %id{{{_format_uuid(props_type_uuid)}}}
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
\t\ts %TypeName{{"ezHairStrandAssetProperties"}}
\t\tu3 %TypeVersion{{1}}
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
}}
"""

    try:
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    except Exception:
        return False
