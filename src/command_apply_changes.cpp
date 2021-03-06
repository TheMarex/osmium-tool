/*

Osmium -- OpenStreetMap data manipulation command line tool
http://osmcode.org/osmium

Copyright (C) 2013-2016  Jochen Topf <jochen@topf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <boost/function_output_iterator.hpp>
#include <boost/program_options.hpp>

#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/object_pointer_collection.hpp>
#include <osmium/osm/object_comparisons.hpp>

#include "command_apply_changes.hpp"

bool CommandApplyChanges::setup(const std::vector<std::string>& arguments) {
    po::options_description opts_cmd{"COMMAND OPTIONS"};
    opts_cmd.add_options()
    ("simplify,s", "Simplify change")
    ("remove-deleted,r", "Remove deleted objects from output")
    ;

    po::options_description opts_common{add_common_options()};
    po::options_description opts_input{add_single_input_options()};
    po::options_description opts_output{add_output_options()};

    po::options_description hidden;
    hidden.add_options()
    ("input-filename", po::value<std::string>(), "OSM input file")
    ("change-filenames", po::value<std::vector<std::string>>(), "OSM change input files")
    ;

    po::options_description desc;
    desc.add(opts_cmd).add(opts_common).add(opts_input).add(opts_output);

    po::options_description parsed_options;
    parsed_options.add(desc).add(hidden);

    po::positional_options_description positional;
    positional.add("input-filename", 1);
    positional.add("change-filenames", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(arguments).options(parsed_options).positional(positional).run(), vm);
    po::notify(vm);

    setup_common(vm, desc);
    setup_input_file(vm);
    setup_output_file(vm);

    if (vm.count("change-filenames")) {
        m_change_filenames = vm["change-filenames"].as<std::vector<std::string>>();
    }

    if (vm.count("simplify")) {
        m_simplify_change = true;
    }

    if (vm.count("remove-deleted")) {
        m_remove_deleted = true;
    }

    return true;
}

void CommandApplyChanges::show_arguments() {
    m_vout << "  input data file name: " << m_input_filename << "\n";
    m_vout << "  input change file names: \n";
    for (const auto& fn : m_change_filenames) {
        m_vout << "    " << fn << "\n";
    }
    m_vout << "  input format: " << m_input_format << "\n";
    show_output_arguments(m_vout);
}

/**
 *  Copy the first OSM object with a given Id to the output iterator. Keep
 *  track of the Id of each object to do this.
 *
 *  If kd is set to true, it will copy deleted objects, too. Otherwise those
 *  are suppressed.
 *
 *  We are using this functor class instead of a simple lambda, because the
 *  lambda doesn't build on MSVC.
 */
class copy_first_with_id {

    osmium::io::OutputIterator<osmium::io::Writer> out;
    osmium::object_id_type id = 0;
    bool keep_deleted;

public:

    copy_first_with_id(osmium::io::OutputIterator<osmium::io::Writer> oi, bool kd) :
        out(oi),
        keep_deleted(kd) {
    }

    void operator()(const osmium::OSMObject& obj) {
        if (obj.id() != id) {
            if (keep_deleted || obj.visible()) {
                *out = obj;
            }
            id = obj.id();
        }
    }

}; // class copy_first_with_id

bool CommandApplyChanges::run() {
    std::vector<osmium::memory::Buffer> changes;
    osmium::ObjectPointerCollection objects;

    m_vout << "Reading change file contents...\n";

    for (const std::string& change_file_name : m_change_filenames) {
        osmium::io::Reader reader(change_file_name, osmium::osm_entity_bits::object);
        while (osmium::memory::Buffer buffer = reader.read()) {
            osmium::apply(buffer, objects);
            changes.push_back(std::move(buffer));
        }
        reader.close();
    }

    m_vout << "Opening input file...\n";
    osmium::io::Reader reader(m_input_file, osmium::osm_entity_bits::object);

    osmium::io::Header header = reader.header();
    header.set("generator", m_generator);

    m_vout << "Opening output file...\n";
    osmium::io::Writer writer(m_output_file, header, m_output_overwrite, m_fsync);
    auto out = osmium::io::make_output_iterator(writer);

    auto input = osmium::io::make_input_iterator_range<osmium::OSMObject>(reader);

    if (m_simplify_change) {
        // If the --simplify option was given we sort with the
        // largest version of each object first and then only
        // copy this last version of any object to the output_buffer.
        m_vout << "Sorting change data...\n";
        objects.sort(osmium::object_order_type_id_reverse_version());

        auto output_it = boost::make_function_output_iterator(
                            copy_first_with_id(out, !m_remove_deleted)
        );

        m_vout << "Applying changes and writing them to output...\n";
        std::set_union(objects.begin(),
                       objects.end(),
                       input.begin(),
                       input.end(),
                       output_it,
                       osmium::object_order_type_id_reverse_version());
    } else {
        // If the --simplify option was not given, this
        // is a straightforward sort of the change files
        // and then a merge with the input file.
        m_vout << "Sorting change data...\n";
        objects.sort(osmium::object_order_type_id_version());

        m_vout << "Applying changes and writing them to output...\n";
        std::set_union(objects.begin(),
                       objects.end(),
                       input.begin(),
                       input.end(),
                       out);
    }

    writer.close();
    reader.close();

    show_memory_used();
    m_vout << "Done.\n";

    return true;
}

namespace {

    const bool register_apply_changes_command = CommandFactory::add("apply-changes", "Apply OSM change files to OSM data file", []() {
        return new CommandApplyChanges();
    });

}

