
# NAME

osmium-fileinfo - show information about an OSM file


# SYNOPSIS

**osmium fileinfo** \[*OPTIONS*\] *OSM-FILE*


# DESCRIPTION

Shows various information about OSM files such as the file type, bounding boxes
in the header, etc.

This command will usually only read the file header. Use the **--extended**
option to show more information.

Normally this command will output the data in human readable form. If the
**-j**, **--json** option is used, the output will be in JSON format instead.

If the **-g**, **--get** option is used, only the value of the named variable
will be printed.

The output is split into three sections:

File
:   This section shows the information available without opening the
    file itself. It contains the file name, the format deduced from the
    file name, the compression used and the size of the file in bytes.

Header
:   This section shows the information available from the header of
    the file (if available, OPL files have no header). Any available
    bounding boxes are shown as well as header options such as the
    generator and file format version.

Data
:   This section shows the information available from reading the whole
    file. It is only shown if the **--extended** option was used. It
    shows the actual bounding box calculated from the nodes in the file,
    the first and last timestamp of all objects in the file, a CRC32
    checksum of the data in the file, the number of changesets, nodes,
    ways, and relations found in the file, whether the objects in the
    file were ordered by type (nodes, then ways, then relations) and
    id, and whether there were multiple versions of the same object in
    the file (history files and change files can have that).


# OPTIONS

-e, --extended
:   Read the complete file and show additional information. The default
    is to read only the header of the file.

-g, --get=VARIABLE
:   Get value of VARIABLE. Can not be used together with --json.

-G, --show-variables
:   Show a list of all variable names.

-j, --json
:   Output in JSON format. Can not be used together with --get.

@MAN_COMMON_OPTIONS@
@MAN_INPUT_OPTIONS@

# VARIABLES

The following variables are available:

    file.name - STRING
    file.format - STRING: XML|PBF
    file.compression - STRING: none|bzip2|gzip
    file.size - INTEGER (0 for stdout)
    header.with_history - BOOL (yes|no)
    header.option.generator - STRING
    header.option.version - STRING
    header.option.pbf_dense_nodes - BOOL (yes|no)
    header.option.osmosis_replication_timestamp - STRING with TIMESTAMP
    header.option.osmosis_replication_sequence_number - INTEGER
    header.option.osmosis_replication_base_url - STRING
    data.bbox - BOX
        (in JSON as nested ARRAY with coordinates)
    data.timestamp.first - STRING with TIMESTAMP
    data.timestamp.last - STRING wih TIMESTAMP
    data.objects_ordered - BOOL (yes|no)
    data.multiple_versions - STRING (yes|no|unknown)
        (in JSON as BOOL and missing if "unknown")
    data.crc32 - STRING with 8 hex digits
    data.count.nodes - INTEGER
    data.count.ways - INTEGER
    data.count.relations - INTEGER
    data.count.changesets - INTEGER
    data.maxid.nodes - INTEGER
    data.maxid.ways - INTEGER
    data.maxid.relations - INTEGER
    data.maxid.changesets - INTEGER

All timestamps are in the usual OSM ISO format `yy-mm-ddThh::mm::ssZ`. Boxes
are in the format `(xmin, ymin, xmax, ymax)`.


# DIAGNOSTICS

**osmium fileinfo** exits with exit code

0
  ~ if everything went alright,

1
  ~ if there was an error processing the data, or

2
  ~ if there was a problem with the command line arguments.


# MEMORY USAGE

**osmium fileinfo** does all its work on the fly and doesn't keep much data in
main memory.


# SEE ALSO

* **osmium**(1), **osmium-file-formats**(5)
* [Osmium website](http://osmcode.org/osmium)

