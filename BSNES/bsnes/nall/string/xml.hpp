#ifdef NALL_STRING_INTERNAL_HPP

//XML v1.0 subset parser
//revision 0.01

namespace nall {
namespace XML {

struct Node {
  string name;
  string data;
  bool attribute;
  vector<Node*> children;

  inline bool exists() const {
    return !name.empty();
  }

  inline bool isName(char c) const {
    if(c >= 'A' && c <= 'Z') return true;
    if(c >= 'a' && c <= 'z') return true;
    if(c >= '0' && c <= '9') return true;
    if(c == '.' || c == '_') return true;
    if(c == '?') return true;
    return false;
  }

  inline bool isWhitespace(char c) const {
    if(c ==  ' ' || c == '\t') return true;
    if(c == '\r' || c == '\n') return true;
    return false;
  }

  //copy part of string from source document into target string; decode markup while copying
  inline void copy(string &target, const char *source, unsigned length) {
    target.reserve(length + 1);

    #if defined(NALL_XML_LITERAL)
    memcpy(target(), source, length);
    target[length] = 0;
    return;
    #endif

    char *output = target();
    while(length) {
      if(*source == '&') {
        if(!memcmp(source, "&lt;",   4)) { *output++ = '<';  source += 4; length -= 4; continue; }
        if(!memcmp(source, "&gt;",   4)) { *output++ = '>';  source += 4; length -= 4; continue; }
        if(!memcmp(source, "&amp;",  5)) { *output++ = '&';  source += 5; length -= 5; continue; }
        if(!memcmp(source, "&apos;", 6)) { *output++ = '\''; source += 6; length -= 6; continue; }
        if(!memcmp(source, "&quot;", 6)) { *output++ = '\"'; source += 6; length -= 6; continue; }
      }

      if(attribute == false && source[0] == '<' && source[1] == '!') {
        //comment
        if(!memcmp(source, "<!--", 4)) {
          source += 4, length -= 4;
          while(memcmp(source, "-->", 3)) source++, length--;
          source += 3, length -= 3;
          continue;
        }

        //CDATA
        if(!memcmp(source, "<![CDATA[", 9)) {
          source += 9, length -= 9;
          while(memcmp(source, "]]>", 3)) *output++ = *source++, length--;
          source += 3, length -= 3;
          continue;
        }
      }

      *output++ = *source++, length--;
    }
    *output = 0;
  }

  inline bool parseExpression(const char *&p) {
    if(*(p + 1) != '!') return false;

    //comment
    if(!memcmp(p, "<!--", 4)) {
      while(*p && memcmp(p, "-->", 3)) p++;
      if(!*p) throw "unclosed comment";
      p += 3;
      return true;
    }

    //CDATA
    if(!memcmp(p, "<![CDATA[", 9)) {
      while(*p && memcmp(p, "]]>", 3)) p++;
      if(!*p) throw "unclosed CDATA";
      p += 3;
      return true;
    }

    //DOCTYPE
    if(!memcmp(p, "<!DOCTYPE", 9)) {
      unsigned counter = 0;
      do {
        char n = *p++;
        if(!n) throw "unclosed DOCTYPE";
        if(n == '<') counter++;
        if(n == '>') counter--;
      } while(counter);
      return true;
    }

    return false;
  }

  //returns true if tag closes itself (<tag/>); false if not (<tag>)
  inline bool parseHead(const char *&p) {
    //parse name
    const char *nameStart = ++p;  //skip '<'
    while(isName(*p)) p++;
    const char *nameEnd = p;
    copy(name, nameStart, nameEnd - nameStart);
    if(name.empty()) throw "missing element name";

    //parse attributes
    while(*p) {
      while(isWhitespace(*p)) p++;
      if(!*p) throw "unclosed attribute";
      if(*p == '?' || *p == '/' || *p == '>') break;

      //parse attribute name
      Node *attribute = new Node;
      children.append(attribute);
      attribute->attribute = true;

      const char *nameStart = p;
      while(isName(*p)) p++;
      const char *nameEnd = p;
      copy(attribute->name, nameStart, nameEnd - nameStart);
      if(attribute->name.empty()) throw "missing attribute name";

      //parse attribute data
      if(*p++ != '=') throw "missing attribute value";
      char terminal = *p++;
      if(terminal != '\'' && terminal != '\"') throw "attribute value not quoted";
      const char *dataStart = p;
      while(*p && *p != terminal) p++;
      if(!*p) throw "missing attribute data terminal";
      const char *dataEnd = p++;  //skip closing terminal

      copy(attribute->data, dataStart, dataEnd - dataStart);
    }

    //parse closure
    if(*p == '?' && *(p + 1) == '>') { p += 2; return true; }
    if(*p == '/' && *(p + 1) == '>') { p += 2; return true; }
    if(*p == '>') { p += 1; return false; }
    throw "invalid element tag";
  }

  //parse element and all of its child elements
  inline void parseElement(const char *&p) {
    Node *node = new Node;
    children.append(node);
    if(node->parseHead(p) == true) return;
    node->parse(p);
  }

  //return true if </tag> matches this node's name
  inline bool parseClosureElement(const char *&p) {
    if(p[0] != '<' || p[1] != '/') return false;
    p += 2;
    const char *nameStart = p;
    while(*p && *p != '>') p++;
    if(*p != '>') throw "unclosed closure element";
    const char *nameEnd = p++;
    if(memcmp(name, nameStart, nameEnd - nameStart)) throw "closure element name mismatch";
    return true;
  }

  //parse contents of an element
  inline void parse(const char *&p) {
    const char *dataStart = p, *dataEnd = p;

    while(*p) {
      while(*p && *p != '<') p++;
      if(!*p) break;
      dataEnd = p;
      if(parseClosureElement(p) == true) break;
      if(parseExpression(p) == true) continue;
      parseElement(p);
    }

    copy(data, dataStart, dataEnd - dataStart);
  }

  inline void reset() {
    for(auto &child : children) delete child;
    children.reset();
  }

  struct iterator {
    inline bool operator!=(const iterator &source) const { return index != source.index; }
    inline Node& operator*() { return *node.children[index]; }
    inline iterator& operator++() { index++; return *this; }
    inline iterator(const Node &node, unsigned index) : node(node), index(index) {}
  private:
    const Node &node;
    unsigned index;
  };

  inline iterator begin() { return iterator(*this, 0); }
  inline iterator end() { return iterator(*this, children.size()); }
  inline const iterator begin() const { return iterator(*this, 0); }
  inline const iterator end() const { return iterator(*this, children.size()); }

  inline Node& operator[](const char *name) {
    for(auto &node : *this) {
      if(node.name == name) return node;
    }
    static Node node;
    return node;
  }

  inline Node() : attribute(false) {}
  inline ~Node() { reset(); }

  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;
};

struct Document : Node {
  string error;

  inline bool load(const char *document) {
    if(document == nullptr) return false;
    reset();
    try {
      parse(document);
    } catch(const char *error) {
      reset();
      this->error = error;
      return false;
    }
    return true;
  }

  inline Document() {}
  inline Document(const char *document) { load(document); }
};

}
}

#endif
