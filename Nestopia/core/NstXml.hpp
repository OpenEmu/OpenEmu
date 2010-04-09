////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef NST_XML_H
#define NST_XML_H

#include <cstring>
#include <iosfwd>

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Xml
		{
			typedef word utfchar;
			typedef const word* utfstring;

			static inline int ToChar(idword);
			static inline wchar_t ToWideChar(idword);

			class BaseNode
			{
			public:

				enum In {IN};
				enum Out {OUT};

			private:

				static utfchar ParseReference(utfstring&,utfstring);

				static wchar_t* SetType(wchar_t* NST_RESTRICT,utfstring,utfstring,In);
				static wchar_t* SetType(wchar_t* NST_RESTRICT,wcstring,wcstring,Out);

				static wchar_t* SetValue(wchar_t* NST_RESTRICT,utfstring,utfstring,In);
				static wchar_t* SetValue(wchar_t* NST_RESTRICT,wcstring,wcstring,Out);

			public:

				struct Attribute
				{
					template<typename T,typename U>
					Attribute(T,T,T,T,U);

					~Attribute();

					wcstring const type;
					wcstring const value;
					Attribute* next;
				};

				template<typename T,typename U>
				BaseNode(T,T,U);

				~BaseNode();

				template<typename T,typename U>
				void SetValue(T,T,U);

				void AddAttribute(utfstring,utfstring,utfstring,utfstring);

				wcstring const type;
				wcstring value;
				Attribute* attribute;
				BaseNode* child;
				BaseNode* sibling;
			};

			static bool IsEqual(wcstring,wcstring);
			static bool IsEqualNonCase(wcstring,wcstring);
			static long ToSigned(wcstring,uint,wcstring*);
			static ulong ToUnsigned(wcstring,uint,wcstring*);

		public:

			Xml();
			~Xml();

			class Node;

			class Attribute : public ImplicitBool<Attribute>
			{
			public:

				long GetSignedValue(uint=0) const;
				long GetSignedValue(wcstring&,uint=0) const;

				ulong GetUnsignedValue(uint=0) const;
				ulong GetUnsignedValue(wcstring&,uint=0) const;

			private:

				friend class Node;

				BaseNode::Attribute* attribute;

				Attribute(BaseNode::Attribute* a)
				: attribute(a) {}

			public:

				bool operator ! () const
				{
					return !attribute;
				}

				wcstring GetType() const
				{
					return attribute ? attribute->type : L"";
				}

				wcstring GetValue() const
				{
					return attribute ? attribute->value : L"";
				}

				Attribute GetNext() const
				{
					return attribute ? attribute->next : NULL;
				}

				bool IsType(wcstring type) const
				{
					return IsEqual( GetType(), type ? type : L"" );
				}

				bool IsValue(wcstring value) const
				{
					return IsEqualNonCase( GetValue(), value ? value : L"" );
				}
			};

			class Node : public ImplicitBool<Node>
			{
				friend class Xml;

			public:

				dword NumChildren(wcstring=NULL) const;
				dword NumAttributes() const;

				Attribute GetAttribute(dword) const;
				Attribute GetAttribute(wcstring) const;

				Node GetChild(dword) const;
				Node GetChild(wcstring) const;

				Node AddChild(wcstring,wcstring=NULL);
				Node AddSibling(wcstring,wcstring=NULL);
				Attribute AddAttribute(wcstring,wcstring);

				long GetSignedValue(uint=0) const;
				long GetSignedValue(wcstring&,uint=0) const;

				ulong GetUnsignedValue(uint=0) const;
				ulong GetUnsignedValue(wcstring&,uint=0) const;

			private:

				BaseNode* Add(wcstring,wcstring,BaseNode**) const;

				BaseNode* node;

				Node(BaseNode* n)
				: node(n) {}

			public:

				Node()
				: node(NULL) {}

				bool operator ! () const
				{
					return !node;
				}

				bool HasChildren() const
				{
					return node && node->child;
				}

				bool HasNextSibling() const
				{
					return node && node->sibling;
				}

				bool HasAttributes() const
				{
					return node && node->attribute;
				}

				Attribute GetFirstAttribute() const
				{
					return node ? node->attribute : NULL;
				}

				Node GetFirstChild() const
				{
					return node ? node->child : NULL;
				}

				Node GetNextSibling() const
				{
					return node ? node->sibling : NULL;
				}

				wcstring GetType() const
				{
					return node ? node->type : L"";
				}

				wcstring GetValue() const
				{
					return node ? node->value : L"";
				}

				bool IsType(wcstring type) const
				{
					return IsEqual( GetType(), type ? type : L"" );
				}

				bool IsValue(wcstring value) const
				{
					return IsEqualNonCase( GetValue(), value ? value : L"" );
				}
			};

			struct Format
			{
				Format();

				cstring tab;
				cstring newline;
				cstring valueDelimiter;
				bool xmlHeader;
				bool byteOrderMark;
			};

			Node Create(wcstring);
			Node Read(utfstring);
			Node Read(std::istream&);
			void Write(Node,std::ostream&,const Format& = Format()) const;
			void Destroy();

		private:

			enum Tag
			{
				TAG_XML,
				TAG_COMMENT,
				TAG_INSTRUCTION,
				TAG_OPEN,
				TAG_OPEN_CLOSE,
				TAG_CLOSE
			};

			class Input
			{
				static byte* Init(std::istream&,dword&);

				const byte* const stream;
				const dword size;
				dword pos;

			public:

				Input(std::istream&,dword=0);
				~Input();

				inline dword Size() const;

				inline uint ToByte(dword) const;
				inline int  ToChar(dword) const;
				inline uint FromUTF16LE(dword) const;
				inline uint FromUTF16BE(dword) const;

				uint ReadUTF8();
				inline void SetReadPointer(dword);
			};

			class Output
			{
				std::ostream& stream;

				void Write(cstring,uint) const;

			public:

				const Format format;

				Output(std::ostream&,const Format&);

				class Type
				{
					friend class Output;
					wcstring string;

				public:

					inline Type(wcstring);
				};

				class Value
				{
					friend class Output;
					wcstring string;

				public:

					inline Value(wcstring);
				};

				const Output& operator << (char) const;
				const Output& operator << (wchar_t) const;
				const Output& operator << (byte) const;
				const Output& operator << (Type) const;
				const Output& operator << (Value) const;

				inline const Output& operator << (cstring) const;

				template<uint N>
				inline const Output& operator << (const char (&)[N]) const;
			};

			static bool IsVoid(utfchar);
			static bool IsCtrl(utfchar);
			static Tag CheckTag(utfstring);

			static utfstring SkipVoid(utfstring);
			static utfstring RewindVoid(utfstring,utfstring=NULL);
			static utfstring ReadTag(utfstring,BaseNode*&);
			static utfstring ReadValue(utfstring,BaseNode&);
			static utfstring ReadNode(utfstring,Tag,BaseNode*&);
			static void WriteNode(Node,const Output&,uint);

			BaseNode* root;

		public:

			Node GetRoot() const
			{
				return root;
			}
		};
	}
}

#endif
