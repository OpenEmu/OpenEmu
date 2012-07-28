//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2011 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: FSNode.hxx 2250 2011-06-09 14:00:30Z stephena $
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef FS_NODE_HXX
#define FS_NODE_HXX

#include <algorithm>

/*
 * The API described in this header is meant to allow for file system browsing in a
 * portable fashions. To this ends, multiple or single roots have to be supported
 * (compare Unix with a single root, Windows with multiple roots C:, D:, ...).
 *
 * To this end, we abstract away from paths; implementations can be based on
 * paths (and it's left to them whether / or \ or : is the path separator :-);
 * but it is also possible to use inodes or vrefs (MacOS 9) or anything else.
 *
 * NOTE: Backends still have to provide a way to extract a path from a FSIntern
 *
 * You may ask now: "isn't this cheating? Why do we go through all this when we use
 * a path in the end anyway?!?".
 * Well, for once as long as we don't provide our own file open/read/write API, we
 * still have to use fopen(). Since all our targets already support fopen(), it should
 * be possible to get a fopen() compatible string for any file system node.
 *
 * Secondly, with this abstraction layer, we still avoid a lot of complications based on
 * differences in FS roots, different path separators, or even systems with no real
 * paths (MacOS 9 doesn't even have the notion of a "current directory").
 * And if we ever want to support devices with no FS in the classical sense (Palm...),
 * we can build upon this.
 */
 
/* 
 * TODO - Instead of starting with getRoot(), we should rather add a getDefaultDir()
 * call that on Unix might return the current dir or the users home dir...
 * i.e. the root dir is usually not the best starting point for browsing.
 */

#include "Array.hxx"
#include "SharedPtr.hxx"

class FilesystemNode;
class AbstractFilesystemNode;

/**
 * List of multiple file system nodes. E.g. the contents of a given directory.
 * This is subclass instead of just a typedef so that we can use forward
 * declarations of it in other places.
 */
class FSList : public Common::Array<FilesystemNode> { };

/**
 * FilesystemNode provides an abstraction for file paths, allowing for portable
 * file system browsing. To this ends, multiple or single roots have to be supported
 * (compare Unix with a single root, Windows with multiple roots C:, D:, ...).
 *
 * To this end, we abstract away from paths; implementations can be based on
 * paths (and it's left to them whether / or \ or : is the path separator :-);
 * but it is also possible to use inodes or vrefs (MacOS 9) or anything else.
 *
 * NOTE: Backends still have to provide a way to extract a path from a FSIntern
 *
 * You may ask now: "isn't this cheating? Why do we go through all this when we use
 * a path in the end anyway?!?".
 * Well, for once as long as we don't provide our own file open/read/write API, we
 * still have to use fopen(). Since all our targets already support fopen(), it should
 * be possible to get a fopen() compatible string for any file system node.
 *
 * Secondly, with this abstraction layer, we still avoid a lot of complications based on
 * differences in FS roots, different path separators, or even systems with no real
 * paths (MacOS 9 doesn't even have the notion of a "current directory").
 * And if we ever want to support devices with no FS in the classical sense (Palm...),
 * we can build upon this.
 *
 * This class acts as a wrapper around the AbstractFilesystemNode class defined in backends/fs.
 */
class FilesystemNode
{
  public:
    /**
     * Flag to tell listDir() which kind of files to list.
     */
    enum ListMode {
      kListFilesOnly = 1,
      kListDirectoriesOnly = 2,
      kListAll = 3
    };

    /**
     * Create a new pathless FilesystemNode. Since there's no path associated
     * with this node, path-related operations (i.e. exists(), isDirectory(),
     * getPath()) will always return false or raise an assertion.
     */
    FilesystemNode();

    /**
     * Create a new FilesystemNode referring to the specified path. This is
     * the counterpart to the path() method.
     *
     * If path is empty or equals "." or "~", then a node representing the
     * "home directory" will be created. If that is not possible (since e.g. the
     * operating system doesn't support the concept), some other directory is
     * used (usually the root directory).
     */
    explicit FilesystemNode(const string& path);

    virtual ~FilesystemNode() {}

    /**
     * Compare the name of this node to the name of another. Directories
     * go before normal files.
     */
    bool operator<(const FilesystemNode& node) const;

    /**
     * Indicates whether the object referred by this path exists in the filesystem or not.
     *
     * @return bool true if the path exists, false otherwise.
     */
    virtual bool exists() const;

    /**
     * Return a list of child nodes of this directory node. If called on a node
     * that does not represent a directory, false is returned.
     *
     * @return true if successful, false otherwise (e.g. when the directory does not exist).
     */
    virtual bool getChildren(FSList &fslist, ListMode mode = kListDirectoriesOnly, bool hidden = false) const;

    /**
     * Return a human readable string for this node, usable for display (e.g.
     * in the GUI code). Do *not* rely on it being usable for anything else,
     * like constructing paths!
     *
     * @return the display name
     */
    virtual string getDisplayName() const;

    /**
     * Return a string representation of the name of the file. This is can be
     * used e.g. by detection code that relies on matching the name of a given
     * file. But it is *not* suitable for use with fopen / File::open, nor
     * should it be archived.
     *
     * @return the file name
     */
    virtual string getName() const;

    /**
     * Return a string representation of the file with the following properties:
     *  1) can be passed to fopen() if fqn is true
     *  2) contains the '~' symbol (if applicable), and is suitable for archiving
     *     (i.e. writing to the config file) if fqn is false
     *
     * This will usually be a 'path' (hence the name of the method), but can
     * be anything that fulfills the above criterions.
     *
     * @note Do not assume that this string contains (back)slashes or any
     *       other kind of 'path separators'.
     *
     * @return the 'path' represented by this filesystem node
     */
    virtual string getPath(bool fqn = true) const;

    /**
     * Determine whether this node has a parent.
     */
    bool hasParent() const;

    /**
     * Get the parent node of this node. If this node has no parent node,
     * then it returns a duplicate of this node.
     */
    FilesystemNode getParent() const;

    /**
     * Indicates whether the path refers to a directory or not.
     *
     * @todo Currently we assume that a node that is not a directory
     * automatically is a file (ignoring things like symlinks or pipes).
     * That might actually be OK... but we could still add an isFile method.
     * Or even replace isDirectory by a getType() method that can return values like
     * kDirNodeType, kFileNodeType, kInvalidNodeType.
     */
    virtual bool isDirectory() const;

    /**
     * Indicates whether the object referred by this path can be read from or not.
     *
     * If the path refers to a directory, readability implies being able to read
     * and list the directory entries.
     *
     * If the path refers to a file, readability implies being able to read the
     * contents of the file.
     *
     * @return bool true if the object can be read, false otherwise.
     */
    virtual bool isReadable() const;

    /**
     * Indicates whether the object referred by this path can be written to or not.
     *
     * If the path refers to a directory, writability implies being able to modify
     * the directory entry (i.e. rename the directory, remove it or write files inside of it).
     *
     * If the path refers to a file, writability implies being able to write data
     * to the file.
     *
     * @return bool true if the object can be written to, false otherwise.
     */
    virtual bool isWritable() const;

  private:
    Common::SharedPtr<AbstractFilesystemNode> _realNode;
    FilesystemNode(AbstractFilesystemNode* realNode);
};


/**
 * Abstract file system node.  Private subclasses implement the actual
 * functionality.
 *
 * Most of the methods correspond directly to methods in class FSNode,
 * so if they are not documented here, look there for more information about
 * the semantics.
 */

typedef Common::Array<AbstractFilesystemNode *>	AbstractFSList;

class AbstractFilesystemNode
{
  protected:
    friend class FilesystemNode;
    typedef FilesystemNode::ListMode ListMode;

  public:
    /**
     * Destructor.
     */
    virtual ~AbstractFilesystemNode() {}

    /*
     * Indicates whether the object referred by this path exists in the filesystem or not.
     */
    virtual bool exists() const = 0;

    /**
     * Return a list of child nodes of this directory node. If called on a node
     * that does not represent a directory, false is returned.
     *
     * @param list List to put the contents of the directory in.
     * @param mode Mode to use while listing the directory.
     * @param hidden Whether to include hidden files or not in the results.
     *
     * @return true if succesful, false otherwise (e.g. when the directory does not exist).
     */
    virtual bool getChildren(AbstractFSList& list, ListMode mode, bool hidden) const = 0;

    /**
     * Returns a human readable path string.
     *
     * @note By default, this method returns the value of getName().
     */
    virtual string getDisplayName() const { return getName(); }

    /**
     * Returns the last component of the path pointed by this FilesystemNode.
     *
     * Examples (POSIX):
     *			/foo/bar.txt would return /bar.txt
     *			/foo/bar/    would return /bar/
     *
     * @note This method is very architecture dependent, please check the concrete implementation for more information.
     */
    virtual string getName() const = 0;

    /**
     * Returns the 'path' of the current node, usable in fopen() or 
       containing '~' and for archiving.
     */
    virtual string getPath(bool fqn = true) const = 0;

    /**
     * Indicates whether this path refers to a directory or not.
     */
    virtual bool isDirectory() const = 0;

    /**
     * Indicates whether the object referred by this path can be read from or not.
     *
     * If the path refers to a directory, readability implies being able to read
     * and list the directory entries.
     *
     * If the path refers to a file, readability implies being able to read the
     * contents of the file.
     *
     * @return bool true if the object can be read, false otherwise.
     */
    virtual bool isReadable() const = 0;

    /**
     * Indicates whether the object referred by this path can be written to or not.
     *
     * If the path refers to a directory, writability implies being able to modify
     * the directory entry (i.e. rename the directory, remove it or write files inside of it).
     *
     * If the path refers to a file, writability implies being able to write data
     * to the file.
     *
     * @return bool true if the object can be written to, false otherwise.
     */
    virtual bool isWritable() const = 0;

    /* TODO:
    bool isFile();
    */

    /**
      Create a directory from the given path.
     */
    static bool makeDir(const string& path);

    /**
      Rename the given file with a new name.
     */
    static bool renameFile(const string& oldfile, const string& newfile);

    /**
      Create an absolute pathname from the given path (if it isn't already
      absolute), pre-pending 'startpath' when necessary.  If the path doesn't
      have an extension matching 'ext', append it to the path.
     */
    static string getAbsolutePath(const string& p, const string& startpath,
                                  const string& ext);

  protected:
    /**
     * The parent node of this directory.
     * The parent of the root is the root itself.
     */
    virtual AbstractFilesystemNode* getParent() const = 0;

    /**
     * Returns a node representing the "home directory".
     *
     * On Unix, this will be the value of $HOME.
     * On Windows, it will be the 'My Documents' folder.
     * Otherwise, it should just return the same value as getRoot().
     */
    static AbstractFilesystemNode* makeHomeDirectoryFileNode();

    /**
     * Construct a node based on a path; the path is in the same format as it
     * would be for calls to fopen().
     *
     * Furthermore getNodeForPath(oldNode.path()) should create a new node
     * identical to oldNode. Hence, we can use the "path" value for persistent
     * storage e.g. in the config file.
     *
     * @param path The path string to create a FilesystemNode for.
     */
    static AbstractFilesystemNode* makeFileNodePath(const string& path);

    /**
     * Returns a special node representing the filesystem root.
     * The starting point for any file system browsing.
     *
     * On Unix, this will be simply the node for / (the root directory).
     * On Windows, it will be a special node which "contains" all drives (C:, D:, E:).
     */
    static AbstractFilesystemNode* makeRootFileNode();
};

#endif
