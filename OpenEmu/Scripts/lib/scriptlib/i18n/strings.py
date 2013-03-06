# 
# Copyright (c) 2012 FadingRed LLC
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 

from util import *
import codecs
import sys
import os
import re

class Strings(object):
  def __init__(self, name, lang, config, resources=None):
    self.name = clean_name(name)
    self.lang = clean_lang(lang)
    self.config = config
    self._resources = resources
    self._mtime = None
    self._translation = None
    self._index = {}
  
  def path(self):
    resources = self._resources or self.config.resources.split(':')[0]
    lang = os.path.join(resources, '%s.lproj' % self.lang)
    path = os.path.join(lang, '%s.strings' % self.name)
    return path
  
  def ensure_dir(self):
    'Create the directory to hold the path if needed'
    directory = os.path.dirname(self.path())
    if not os.path.exists(directory):
      os.mkdir(directory)
  
  def modification_time(self):
    if self._mtime == None:
      try: self._mtime = os.path.getmtime(self.path())
      except OSError: self._mtime = 0
    return self._mtime

  def translation(self):
    if not self._translation:
      self._create_translation()
    return self._translation
  
  def index(self, normalized=True):
    if not self._index:
      self._create_translation()
    if not normalized:
      return self._uindex
    return self._index
  
  def _create_translation(self):
    resources = self._resources or self.config.resources.split(':')[0]
    langdir = os.path.join(resources, '%s.lproj' % self.lang)
    base = self.config.lang == self.lang
    self._translation = Parser(self.name, langdir, base=base)
    self._index = {}
    self._uindex = {}
    for key, value in self._translation.index.items():
      self._uindex[value.key] = value
      if base:
        # since we're the base language, strip out the extra argument information
        # on the key and the translation and if they're not equal, then just use
        # the translation value as the key value (this will be true for interface files)
        FMTARG = r'%\d\$(@|d|D|i|u|U|hi|hu|qi|qu|x|X|qx|qX|o|O|f|e|E|g|G|s|S|p|L|a|A|F|z|t|j)'
        FMTSUB = r'%\1'
        if re.sub(FMTARG, FMTSUB, value.key) != re.sub(FMTARG, FMTSUB, value.trans):
          value.key = value.trans
      self._index[value.key] = value
  
  def normalize(self, source=None):
    """
    Normalizes strings to an expected format. If given, it will use source
    as the source of normalized keys to use. This is convenient for migrating
    xib files over to multiple strings files since the real keys are object
    identifiers in the xib. This allows you to use the source language strings
    as the source for normalization and actually get the source language key
    for other languages.
    """
    lines = []
    if not source: source = self
    for key in sorted(self.index().keys(), key=unicode.lower):
      line_trans = self.index()[key].trans
      try: line_key = source.index()[key].key
      except KeyError: line_key = source.index(normalized=False)[key].key
      lines += [
        '/* No comment provided by engineer. */\n',
        '"%s" = "%s";\n\n' % (line_key, line_trans),
      ]
    os.unlink(self.path())
    if lines:
      result = codecs.open(self.path(), 'wb', encoding='utf-16')
      result.writelines(lines)
      result.close()
  
  def integrate_changes(self, strings):
    """
    Integrates changes from the given strings file into this strings file
    and writes the changes out.
    """
    other_index = strings.index()
    translation = self.translation()
    lines = []
    for key in sorted(other_index.keys(), key=unicode.lower):
      try: value = translation.index[key]
      except KeyError: value = other_index[key]
      try: translation_comments = translation.index[key].comments
      except: translation_comments = []
      other_comments = other_index[key].comments or ['No comment provided by engineer.']
      for i in range(0, max(len(translation_comments), len(other_comments))):
        try: comment = other_comments[i]
        except IndexError: comment = translation_comments[i]
        lines.append('/* %s */\n' % comment)
      lines.append('"%s" = "%s";\n\n' % (value.key, value.trans))
    
    try: os.unlink(self.path())
    except OSError: pass
    if lines:
      result = codecs.open(self.path(), 'wb', encoding='utf-16')
      result.writelines(lines)
      result.close()

class Parser(object):
  LINE = r'^"(.*)" = "(.*)";$'
  COMMENT = r'^/\* (.*) \*/$'
  INTERFACE = r'^/\* Class = ".*"; .* ObjectID = ".*"; \*/$'
  
  def __init__(self, name, directory, base=False):
    try:
      strings = codecs.open(os.path.join(directory, '%s.strings' % name), encoding='utf-16')
    except IOError:
      self.lines = []
    else:
      self.lines = strings.readlines()
      strings.close()
    
    linenumber = 0
    self.index = {}
    
    comments = []
    for line in self.lines:
      match = re.search(Parser.LINE, line, re.UNICODE)
      if match:
        key = match.group(1)
        trans = match.group(2)
        self.index[key] = Parser.Translation(key, trans, comments, linenumber)
        comments = []
      else:
        if not re.match(Parser.INTERFACE, line, re.UNICODE):
          match = re.search(Parser.COMMENT, line, re.UNICODE)
          if match: comments.append(match.group(1))
          else: comments = []
      linenumber += 1
  
  @staticmethod
  def cmp(a, b): return cmp(a.linenum, b.linenum)
  
  class Translation(object):
    def __init__(self, key, trans, comments, linenum):
      self.key = key
      self.trans = trans
      self.comments = comments
      self.linenum = linenum
  