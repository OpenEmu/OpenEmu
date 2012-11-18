#include <nall/file.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>
using namespace nall;

static bool cycle_accurate;

struct opcode_t {
  string name;
  lstring args;
  unsigned opcode;
};

void generate(const char *sourceFilename, const char *targetFilename) {
  file fp;
  fp.open(targetFilename, file::mode::write);

  string filedata;
  filedata.readfile(sourceFilename);
  filedata.replace("\r", "");

  lstring block;
  block.split("\n\n", filedata);

  foreach(data, block) {
    lstring lines;
    lines.split("\n", data);

    linear_vector<opcode_t> array;

    unsigned sourceStart = 0;
    foreach(line, lines, currentLine) {
      line.transform("()", "``");
      lstring part;
      part.split("`", line);
      lstring arguments;
      arguments.split(", ", part[1]);

      opcode_t opcode;
      opcode.name = part[0];
      opcode.args = arguments;
      opcode.opcode = hex(arguments[0]);
      array.append(opcode);

      line.rtrim<1>(",");
      if(line.endswith(" {")) {
        line.rtrim<1>("{ ");
        sourceStart = currentLine + 1;
        break;
      }
    }

    if(cycle_accurate == false) {
      foreach(opcode, array) {
        fp.print("case 0x", hex<2>(opcode.opcode), ": {\n");

        for(unsigned n = sourceStart; n < lines.size(); n++) {
          if(lines[n] == "}") break;

          string output;

          if(lines[n].beginswith("  ")) {
            output = lines[n];
          } else {
            lstring part;
            part.split<1>(":", lines[n]);
            output = { "  ", part[1] };
          }

          output.replace("$1", opcode.args[1]);
          output.replace("$2", opcode.args[2]);
          output.replace("$3", opcode.args[3]);
          output.replace("$4", opcode.args[4]);
          output.replace("$5", opcode.args[5]);
          output.replace("$6", opcode.args[6]);
          output.replace("$7", opcode.args[7]);
          output.replace("$8", opcode.args[8]);
          output.replace("end;", "break;");

          fp.print(output, "\n");
        }

        fp.print("  break;\n");
        fp.print("}\n\n");
      }
    } else {
      foreach(opcode, array) {
        fp.print("case 0x", hex<2>(opcode.opcode), ": {\n");
        fp.print("  switch(opcode_cycle++) {\n");

        for(unsigned n = sourceStart; n < lines.size(); n++) {
          if(lines[n] == "}") break;

          bool nextLineEndsCycle = false;
          if(lines[n + 1] == "}") nextLineEndsCycle = true;
          if(lines[n + 1].beginswith("  ") == false) nextLineEndsCycle = true;

          string output;

          if(lines[n].beginswith("  ")) {
            output = { "  ", lines[n] };
          } else {
            lstring part;
            part.split<1>(":", lines[n]);
            fp.print("  case ", (unsigned)decimal(part[0]), ":\n");
            output = { "    ", part[1] };
          }

          output.replace("$1", opcode.args[1]);
          output.replace("$2", opcode.args[2]);
          output.replace("$3", opcode.args[3]);
          output.replace("$4", opcode.args[4]);
          output.replace("$5", opcode.args[5]);
          output.replace("$6", opcode.args[6]);
          output.replace("$7", opcode.args[7]);
          output.replace("$8", opcode.args[8]);
          output.replace("end;", "{ opcode_cycle = 0; break; }");

          fp.print(output, "\n");
          if(nextLineEndsCycle) {
            if(lines[n + 1].beginswith("}")) {
              fp.print("    opcode_cycle = 0;\n");
            }
            fp.print("    break;\n");
          }
        }

        fp.print("  }\n");
        fp.print("  break;\n");
        fp.print("}\n\n");
      }
    }
  }

  fp.close();
}

int main() {
  cycle_accurate = false;
  generate("op_misc.b", "op_misc.cpp");
  generate("op_mov.b",  "op_mov.cpp" );
  generate("op_pc.b",   "op_pc.cpp" );
  generate("op_read.b", "op_read.cpp");
  generate("op_rmw.b",  "op_rmw.cpp" );

  cycle_accurate = true;
  generate("op_misc.b", "opcycle_misc.cpp");
  generate("op_mov.b",  "opcycle_mov.cpp" );
  generate("op_pc.b",   "opcycle_pc.cpp" );
  generate("op_read.b", "opcycle_read.cpp");
  generate("op_rmw.b",  "opcycle_rmw.cpp" );

  return 0;
}
