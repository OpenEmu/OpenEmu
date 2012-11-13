#ifdef NALL_STRING_INTERNAL_HPP

//BML v1.0 parser
//revision 0.05

namespace nall {
namespace BML {

inline static string indent(const char *s, unsigned depth) {
  vector<char> output;
  do {
    for(unsigned n = 0; n < depth; n++) output.append('\t');
    do output.append(*s); while(*s && *s++ != '\n');
  } while(*s);
  return output.data();
}

struct Node {
  cstring name;
  cstring value;

private:
  vector<Node> children;

  inline bool valid(char p) const {  //A-Za-z0-9-.
    return p - 'A' < 26u | p - 'a' < 26u | p - '0' < 10u | p - '-' < 2u;
  }

  inline unsigned parseDepth(char *&p) {
    while(*p == '\n' || *p == '#') {
      while(*p != '\n') *p++ = 0;
      *p++ = 0;  //'\n'
    }
    unsigned depth = 0;
    while(p[depth] == '\t') depth++;
    return depth;
  }

  inline void parseName(char *&p) {
    if(valid(*p) == false) throw "Missing node name";
    name = p;
    while(valid(*p)) p++;
  }

  inline void parseValue(char *&p) {
    char terminal = *p == ':' ? '\n' : ' ';  //':' or '='
    *p++ = 0;
    value = p;
    while(*p && *p != terminal && *p != '\n') p++;
  }

  inline void parseBlock(char *&p, unsigned depth) {
    value = p;
    char *w = p;
    while(parseDepth(p) > depth) {
      p += depth + 1;
      while(*p && *p != '\n') *w++ = *p++;
      if(*p && *p != '\n') throw "Multi-line value missing line feed";
      *w++ = *p;
    }
    *(w - 1) = 0;  //'\n'
  }

  inline void parseLine(char *&p) {
    unsigned depth = parseDepth(p);
    while(*p == '\t') p++;

    parseName(p);
    bool multiLine = *p == '~';
    if(multiLine) *p++ = 0;
    else if(*p == ':' || *p == '=') parseValue(p);
    if(*p && *p != ' ' && *p != '\n') throw "Invalid character encountered";

    while(*p == ' ') {
      *p++ = 0;
      Node node;
      node.parseName(p);
      if(*p == ':' || *p == '=') node.parseValue(p);
      if(*p && *p != ' ' && *p != '\n') throw "Invalid character after node";
      if(*p == '\n') *p++ = 0;
      children.append(node);
    }

    if(multiLine) return parseBlock(p, depth);

    while(parseDepth(p) > depth) {
      Node node;
      node.parseLine(p);
      children.append(node);
    }
  }

  inline void parse(char *&p) {
    while(*p) {
      Node node;
      node.parseLine(p);
      children.append(node);
    }
  }

public:
  inline Node& operator[](const char *name) {
    for(auto &node : children) {
      if(node.name == name) return node;
    }
    static Node node;
    node.name = nullptr;
    return node;
  }

  inline bool exists() const { return name; }
  unsigned size() const { return children.size(); }
  Node* begin() { return children.begin(); }
  Node* end() { return children.end(); }
  const Node* begin() const { return children.begin(); }
  const Node* end() const { return children.end(); }
  inline Node() : name(""), value("") {}
  friend class Document;
};

struct Document : Node {
  cstring error;

  inline bool load(const char *document) {
    if(document == nullptr) return false;
    this->document = strdup(document);
    char *p = this->document;
    try {
      this->error = nullptr;
      parse(p);
    } catch(const char *error) {
      this->error = error;
      free(this->document);
      this->document = nullptr;
      children.reset();
      return false;
    }
    return true;
  }

  inline Document(const char *document = "") : document(nullptr), error(nullptr) { if(*document) load(document); }
  inline ~Document() { if(document) free(document); }

private:
  char *document;
};

}
}

#endif
