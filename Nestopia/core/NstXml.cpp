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

#include <cstdlib>
#include <cwchar>
#include <cerrno>
#include <cstring>
#include <iostream>
#include "NstStream.hpp"
#include "NstVector.hpp"
#include "NstXml.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Xml::Xml()
		: root(NULL) {}

		Xml::~Xml()
		{
			Destroy();
		}

		void Xml::Destroy()
		{
			delete root;
			root = NULL;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Xml::Format::Format()
		:
		tab            ("    "),
		newline        ("\r\n"),
		valueDelimiter (""),
		xmlHeader      (true),
		byteOrderMark  (true)
		{
		}

		inline int Xml::ToChar(idword ch)
		{
			return CHAR_MAX < 0xFF && ch > CHAR_MAX ? ch - (CHAR_MAX-CHAR_MIN+1) : ch;
		}

		inline wchar_t Xml::ToWideChar(idword ch)
		{
			return WCHAR_MAX < 0xFFFF && ch > WCHAR_MAX ? ch - (WCHAR_MAX-WCHAR_MIN+1) : ch;
		}

		byte* Xml::Input::Init(std::istream& stdStream,dword& size)
		{
			byte* data = NULL;

			try
			{
				Stream::In stream( &stdStream );

				size = stream.Length();
				data = new byte [size + 4];

				stream.Read( data, size );
				std::memset( data + size, 0, 4 );
			}
			catch (...)
			{
				if (data)
					delete [] data;

				throw;
			}

			return data;
		}

		Xml::Input::Input(std::istream& s,dword t)
		: stream(Init(s,t)), size(t), pos(0) {}

		Xml::Input::~Input()
		{
			delete [] stream;
		}

		inline dword Xml::Input::Size() const
		{
			return size;
		}

		inline uint Xml::Input::ToByte(dword i) const
		{
			NST_ASSERT( i < size+4 );
			return stream[i];
		}

		inline int Xml::Input::ToChar(dword i) const
		{
			NST_ASSERT( i < size+4 );
			return Xml::ToChar(stream[i]);
		}

		inline uint Xml::Input::FromUTF16BE(dword i) const
		{
			return ToByte(i+1) | ToByte(i+0) << 8;
		}

		inline uint Xml::Input::FromUTF16LE(dword i) const
		{
			return ToByte(i+0) | ToByte(i+1) << 8;
		}

		inline void Xml::Input::SetReadPointer(dword p)
		{
			NST_ASSERT( p <= size );
			pos = p;
		}

		uint Xml::Input::ReadUTF8()
		{
			NST_ASSERT( pos <= size );

			if (uint v = ToByte(pos))
			{
				pos++;

				if (v & 0x80)
				{
					const uint w = ToByte(pos++);

					if ((v & 0xE0) == 0xC0)
					{
						if ((w & 0xC0) != 0x80)
							throw 1;

						v = (v << 6 & 0x7C0) | (w & 0x03F);
					}
					else if ((v & 0xF0) == 0xE0)
					{
						const uint z = ToByte(pos++);

						if ((w & 0xC0) == 0x80)
						{
							if ((z & 0xC0) != 0x80)
								throw 1;

							v = (v << 12 & 0xF000) | (w << 6 & 0x0FC0) | (z & 0x03F);
						}
					}
					else
					{
						throw 1;
					}
				}

				return v;
			}

			return 0;
		}

		Xml::Output::Output(std::ostream& s,const Format& f)
		: stream(s), format(f) {}

		inline Xml::Output::Type::Type(wcstring s)
		: string(s) {}

		inline Xml::Output::Value::Value(wcstring s)
		: string(s) {}

		void Xml::Output::Write(cstring s,uint n) const
		{
			stream.write( s, n );
		}

		const Xml::Output& Xml::Output::operator << (char c) const
		{
			stream.put( c );
			return *this;
		}

		const Xml::Output& Xml::Output::operator << (byte c) const
		{
			return *this << char(Xml::ToChar(c));
		}

		const Xml::Output& Xml::Output::operator << (wchar_t c) const
		{
			const uint v = (WCHAR_MIN < 0 ? (c + (WCHAR_MAX-WCHAR_MIN+1)) & 0xFFFFU : c);

			if (v < 0x80)
			{
				*this << byte( v );
			}
			else if (v < 0x800)
			{
				*this << byte( 0xC0 | (v >> 6 & 0x1F) );
				*this << byte( 0x80 | (v >> 0 & 0x3F) );
			}
			else
			{
				*this << byte( 0xE0 | (v >> 12 & 0x0F) );
				*this << byte( 0x80 | (v >> 6  & 0x3F) );
				*this << byte( 0x80 | (v >> 0  & 0x3F) );
			}

			return *this;
		}

		const Xml::Output& Xml::Output::operator << (Type type) const
		{
			while (const wchar_t c = *type.string++)
				(*this) << c;

			return *this;
		}

		const Xml::Output& Xml::Output::operator << (Value value) const
		{
			while (const wchar_t c = *value.string++)
			{
				switch (c)
				{
					case L'&':

						(*this) << "&amp;";
						break;

					case L'<':

						(*this) << "&lt;";
						break;

					case L'>':

						(*this) << "&gt;";
						break;

					case L'\'':

						(*this) << "&apos;";
						break;

					case L'\"':

						(*this) << "&quot;";
						break;

					default:

						(*this) << c;
						break;
				}
			}

			return *this;
		}

		inline const Xml::Output& Xml::Output::operator << (cstring s) const
		{
			while (const char c = *s++)
				(*this) << c;

			return *this;
		}

		template<uint N>
		inline const Xml::Output& Xml::Output::operator << (const char (&s)[N]) const
		{
			Write( s, N-1 );
			return *this;
		}

		Xml::Node Xml::Read(std::istream& stream)
		{
			Destroy();

			Vector<utfchar> buffer;

			try
			{
				Input input( stream );

				if (input.ToByte(0) == 0xFE && input.ToByte(1) == 0xFF)
				{
					buffer.Resize( input.Size() / 2 );

					for (dword i=0, n=buffer.Size(); i < n; ++i)
						buffer[i] = input.FromUTF16BE( 2 + i * 2 );
				}
				else if (input.ToByte(0) == 0xFF && input.ToByte(1) == 0xFE)
				{
					buffer.Resize( input.Size() / 2 );

					for (dword i=0, n=buffer.Size(); i < n; ++i)
						buffer[i] = input.FromUTF16LE( 2 + i * 2 );
				}
				else
				{
					bool utf8 = (input.ToByte(0) == 0xEF && input.ToByte(1) == 0xBB && input.ToByte(2) == 0xBF);

					if (utf8)
					{
						input.SetReadPointer(3);
					}
					else if (input.ToChar(0) == '<' && input.ToChar(1) == '?')
					{
						for (uint i=2; i < 128 && input.ToChar(i) && input.ToChar(i) != '>'; ++i)
						{
							if
							(
								(input.ToChar( i+0 ) == 'U' || input.ToChar( i+0 ) == 'u') &&
								(input.ToChar( i+1 ) == 'T' || input.ToChar( i+1 ) == 't') &&
								(input.ToChar( i+2 ) == 'F' || input.ToChar( i+2 ) == 'f') &&
								(input.ToChar( i+3 ) == '-' && input.ToChar( i+4 ) == '8')
							)
							{
								utf8 = true;
								break;
							}
						}
					}

					if (utf8)
					{
						buffer.Reserve( input.Size() );

						uint v;

						do
						{
							v = input.ReadUTF8();
							buffer.Append( v );
						}
						while (v);
					}
					else
					{
						buffer.Resize( input.Size() + 1 );

						for (dword i=0, n=buffer.Size(); i < n; ++i)
							buffer[i] = input.ToByte( i );
					}
				}
			}
			catch (...)
			{
				Destroy();
				return NULL;
			}

			return Read( buffer.Begin() );
		}

		Xml::Node Xml::Create(wcstring type)
		{
			Destroy();

			if (type)
			{
				try
				{
					root = new BaseNode( type, type + std::wcslen(type), BaseNode::OUT );
				}
				catch (...)
				{
					Destroy();
				}
			}

			return root;
		}

		Xml::Node Xml::Read(utfstring file)
		{
			Destroy();

			if (file)
			{
				try
				{
					for (utfstring stream = SkipVoid( file ); *stream; )
					{
						switch (const Tag tag = CheckTag( stream ))
						{
							case TAG_XML:

								if (stream != file)
									throw 1;

							case TAG_COMMENT:
							case TAG_INSTRUCTION:

								stream = ReadTag( stream, root );
								break;

							case TAG_OPEN:
							case TAG_OPEN_CLOSE:

								if (!root)
								{
									stream = ReadNode( stream, tag, root );
									break;
								}

							default:

								throw 1;
						}
					}
				}
				catch (...)
				{
					Destroy();
				}
			}

			return root;
		}

		void Xml::Write(const Node node,std::ostream& stream,const Format& format) const
		{
			if (node)
			{
				const Output output( stream, format );

				if (format.byteOrderMark)
					output << byte(0xEF) << byte(0xBB) << byte(0xBF);

				if (format.xmlHeader)
					output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << format.newline;

				WriteNode( node, output, 0 );
			}
		}

		void Xml::WriteNode(const Node node,const Output& output,const uint level)
		{
			for (uint i=level; i; --i)
				output << output.format.tab;

			output << '<' << Output::Type(node.GetType());

			for (Attribute attribute(node.GetFirstAttribute()); attribute; attribute = attribute.GetNext())
			{
				output << ' '
                       << Output::Type(attribute.GetType())
                       << "=\""
                       << Output::Value(attribute.GetValue())
                       << '\"';
			}

			if (node.HasChildren() || *node.GetValue())
			{
				output << '>';

				if (*node.GetValue())
				{
					output << output.format.valueDelimiter
                           << Output::Value(node.GetValue())
                           << output.format.valueDelimiter;
				}

				if (node.HasChildren())
				{
					output << output.format.newline;

					for (Node child(node.GetFirstChild()); child; child=child.GetNextSibling())
						WriteNode( child, output, level+1 );

					for (uint i=level; i; --i)
						output << output.format.tab;
				}

				output << "</" << Output::Type(node.GetType()) << '>';
			}
			else
			{
				output << " />";
			}

			output << output.format.newline;
		}

		Xml::utfstring Xml::ReadNode(utfstring stream,Tag tag,BaseNode*& node)
		{
			NST_ASSERT( node == NULL && tag != TAG_CLOSE );

			stream = ReadTag( stream, node );

			if (tag == TAG_OPEN)
			{
				for (BaseNode** next = &node->child;;)
				{
					if (*stream == '<')
					{
						tag = CheckTag( stream );

						if (tag == TAG_CLOSE)
							break;

						stream = ReadNode( stream, tag, *next );

						if (*next)
							next = &(*next)->sibling;
					}
					else
					{
						stream = ReadValue( stream, *node );
					}
				}

				stream = ReadTag( stream, node );
			}

			return stream;
		}

		Xml::utfstring Xml::ReadTag(utfstring stream,BaseNode*& node)
		{
			NST_ASSERT( *stream == '<' );

			++stream;

			if (*stream == '!')
			{
				if (stream[1] == '-' && stream[2] == '-')
				{
					for (stream += 3; *stream; ++stream)
					{
						if (stream[0] == '-' && stream[1] == '-' && stream[2] == '>')
						{
							stream += 2;
							break;
						}
					}
				}
			}
			else if (*stream == '?')
			{
				while (*++stream)
				{
					if (stream[0] == '?' && stream[1] == '>')
					{
						++stream;
						break;
					}
				}
			}
			else if (node)
			{
				if (*stream++ != '/')
					throw 1;

				for (wcstring type=node->type; *stream; ++stream, ++type)
				{
					if (ToWideChar(*stream) != *type)
					{
						if (*type)
							throw 1;

						stream = SkipVoid( stream );
						break;
					}
				}
			}
			else
			{
				for (utfstring const t=stream; *stream; ++stream)
				{
					if (*stream == '>' || *stream == '/' || IsVoid( *stream ))
					{
						node = new BaseNode( t, stream, BaseNode::IN );
						break;
					}
				}

				for (;;++stream)
				{
					if (*stream == '>')
					{
						break;
					}
					else if (*stream == '/')
					{
						++stream;
						break;
					}
					else if (!IsVoid( *stream ))
					{
						utfstring const t = stream;

						while (*stream && *stream != '=' && !IsVoid( *stream ))
							++stream;

						utfstring const tn = stream;

						stream = SkipVoid( stream );

						if (*stream++ != '=')
							throw 1;

						stream = SkipVoid( stream );

						const utfchar enclosing = *stream++;

						if (enclosing != '\"' && enclosing != '\'')
							throw 1;

						stream = SkipVoid( stream );

						utfstring const v = stream;

						while (*stream && *stream != enclosing)
							++stream;

						if (*stream != enclosing)
							throw 1;

						node->AddAttribute( t, tn, v, RewindVoid(stream,v) );
					}
				}
			}

			if (*stream++ != '>')
				throw 1;

			return SkipVoid( stream );
		}

		Xml::utfstring Xml::ReadValue(utfstring stream,BaseNode& node)
		{
			NST_ASSERT( *stream != '<' && !IsVoid( *stream ) );

			for (utfstring const value = stream; *stream; ++stream)
			{
				if (*stream == '<')
				{
					node.SetValue( value, RewindVoid(stream), BaseNode::IN );
					break;
				}
			}

			return stream;
		}

		bool Xml::IsEqual(wcstring a,wcstring b)
		{
			do
			{
				if (*a != *b)
					return false;
			}
			while (++b, *a++);

			return true;
		}

		bool Xml::IsEqualNonCase(wcstring a,wcstring b)
		{
			do
			{
				if
				(
					(*a >= L'A' && *a <= L'Z' ? L'a' + (*a - L'A') : *a) !=
					(*b >= L'A' && *b <= L'Z' ? L'a' + (*b - L'A') : *b)
				)
					return false;
			}
			while (++b, *a++);

			return true;
		}

		long Xml::ToSigned(wcstring string,uint base,wcstring* end)
		{
			NST_ASSERT( string );

			long value = 0;

			if (*string)
			{
				wchar_t* endptr = NULL;
				value = std::wcstol( string, end ? &endptr : NULL, base );

				if (end)
					*end = (endptr ? endptr : string);

				if (errno == ERANGE)
					value = 0;
			}

			return value;
		}

		ulong Xml::ToUnsigned(wcstring string,uint base,wcstring* end)
		{
			NST_ASSERT( string );

			ulong value = 0;

			if (*string)
			{
				wchar_t* endptr = NULL;
				value = std::wcstoul( string, end ? &endptr : NULL, base );

				if (end)
					*end = (endptr ? endptr : string);

				if (errno == ERANGE)
					value = 0;
			}

			return value;
		}

		bool Xml::IsVoid(utfchar ch)
		{
			switch (ch)
			{
				case ' ':
				case '\r':
				case '\n':
				case '\t':

					return true;
			}

			return false;
		}

		bool Xml::IsCtrl(utfchar ch)
		{
			switch (ch)
			{
				case '\0':
				case '\a':
				case '\b':
				case '\t':
				case '\v':
				case '\n':
				case '\r':
				case '\f':

					return true;
			}

			return false;
		}

		Xml::utfstring Xml::SkipVoid(utfstring stream)
		{
			while (IsVoid( *stream ))
				++stream;

			return stream;
		}

		Xml::utfstring Xml::RewindVoid(utfstring stream,utfstring stop)
		{
			while (stream != stop && IsVoid( stream[-1] ))
				--stream;

			return stream;
		}

		Xml::Tag Xml::CheckTag(utfstring stream)
		{
			if (stream[0] == '<')
			{
				if (stream[1] == '/')
					return TAG_CLOSE;

				if
				(
					stream[1] == '!' &&
					stream[2] == '-' &&
					stream[3] == '-'
				)
					return TAG_COMMENT;

				if (stream[1] == '?')
				{
					if
					(
						stream[2] == 'x' &&
						stream[3] == 'm' &&
						stream[4] == 'l' &&
						IsVoid( stream[5] )
					)
						return TAG_XML;
					else
						return TAG_INSTRUCTION;
				}

				while (*++stream)
				{
					if (*stream == '\"' || *stream == '\'')
					{
						for (const utfchar enclosing = *stream; stream[1]; )
						{
							if (*++stream == enclosing)
								break;
						}
					}
					else if (*stream == '>')
					{
						if (stream[-1] == '/')
							return TAG_OPEN_CLOSE;
						else
							return TAG_OPEN;
					}
				}
			}

			throw 1;
		}

		template<typename T,typename U>
		Xml::BaseNode::BaseNode(T t,T n,U u)
		:
		type      (SetType(new wchar_t [n-t+1],t,n,u)),
		value     (L""),
		attribute (NULL),
		child     (NULL),
		sibling   (NULL)
		{
			if (!type)
				throw 1;
		}

		template<typename T,typename U>
		Xml::BaseNode::Attribute::Attribute(T t,T tn,T v,T vn,U u)
		:
		type  (SetType(new wchar_t [(tn-t)+1+(vn-v)+1],t,tn,u)),
		value (SetValue(const_cast<wchar_t*>(type+(tn-t)+1),v,vn,u)),
		next  (NULL)
		{
		}

		Xml::BaseNode::Attribute::~Attribute()
		{
			delete [] type;
			delete next;
		}

		Xml::BaseNode::~BaseNode()
		{
			delete [] type;

			if (*value)
				delete [] value;

			delete attribute;
			delete child;

			if (BaseNode* node = sibling)
			{
				do
				{
					BaseNode* tmp = node->sibling;
					node->sibling = NULL;
					delete node;
					node = tmp;
				}
				while (node);
			}
		}

		template<typename T,typename U>
		void Xml::BaseNode::SetValue(T v,T vn,U u)
		{
			if (vn-v)
			{
				if (!*value)
					value = SetValue( new wchar_t [vn-v+1], v, vn, u );
				else
					throw 1;
			}
		}

		void Xml::BaseNode::AddAttribute(utfstring t,utfstring tn,utfstring v,utfstring vn)
		{
			if (tn-t)
			{
				Attribute** a = &attribute;

				while (*a)
					a = &(*a)->next;

				(*a) = new Attribute( t, tn, v, vn, BaseNode::IN );
			}
			else if (vn-t)
			{
				throw 1;
			}
		}

		wchar_t* Xml::BaseNode::SetType(wchar_t* NST_RESTRICT dst,utfstring src,utfstring const end,In)
		{
			NST_ASSERT( dst && src && end );

			wchar_t* const ptr = dst;

			while (src != end)
			{
				const utfchar ch = *src++;

				if (!IsCtrl( ch ))
				{
					*dst++ = ToWideChar( ch );
				}
				else
				{
					delete [] ptr;
					return NULL;
				}
			}

			*dst = L'\0';

			return ptr;
		}

		wchar_t* Xml::BaseNode::SetType(wchar_t* NST_RESTRICT dst,wcstring src,wcstring const end,Out)
		{
			NST_ASSERT( dst && src && end );

			wchar_t* const ptr = dst;

			while (src != end)
				*dst++ = *src++;

			*dst = L'\0';

			return ptr;
		}

		wchar_t* Xml::BaseNode::SetValue(wchar_t* NST_RESTRICT dst,utfstring src,utfstring const end,In)
		{
			NST_ASSERT( dst && src && end );

			wchar_t* const ptr = dst;

			while (src != end)
			{
				utfchar ch = *src++;

				if (ch == '&')
					ch = ParseReference( src, end );

				if (!IsCtrl( ch ) || IsVoid( ch ))
				{
					*dst++ = ToWideChar( ch );
				}
				else
				{
					delete [] ptr;
					return NULL;
				}
			}

			*dst = L'\0';

			return ptr;
		}

		wchar_t* Xml::BaseNode::SetValue(wchar_t* NST_RESTRICT dst,wcstring src,wcstring const end,Out)
		{
			NST_ASSERT( dst && src && end );

			wchar_t* const ptr = dst;

			while (src != end)
				*dst++ = *src++;

			*dst = L'\0';

			return ptr;
		}

		Xml::utfchar Xml::BaseNode::ParseReference(utfstring& string,utfstring const end)
		{
			utfstring src = string;

			if (end-src >= 3)
			{
				switch (*src++)
				{
					case '#':

						for (utfstring const offset = src++; src != end; ++src)
						{
							if (*src == ';')
							{
								string = src + 1;

								if (*offset == 'x')
								{
									for (dword ch=0, n=0; ; n += (n < 16 ? 4 : 0))
									{
										const utfchar v = *--src;

										if (v >= '0' && v <= '9')
										{
											ch |= dword(v - '0') << n;
										}
										else if (v >= 'a' && v <= 'f')
										{
											ch |= dword(v - 'a' + 10) << n;
										}
										else if (v >= 'A' && v <= 'F')
										{
											ch |= dword(v - 'A' + 10) << n;
										}
										else
										{
											return src == offset && ch <= 0xFFFF ? ch : '\0';
										}
									}
								}
								else
								{
									for (dword ch=0, n=1; ; n *= (n < 100000 ? 10 : 1))
									{
										const utfchar v = *--src;

										if (v >= '0' && v <= '9')
										{
											ch += (v - '0') * n;
										}
										else
										{
											return src < offset && ch <= 0xFFFF ? ch : '\0';
										}
									}
								}
							}
						}
						break;

					case 'a':

						if (*src == 'm')
						{
							if
							(
								end-src >= 3 &&
								src[1] == 'p' &&
								src[2] == ';'
							)
							{
								string = src + 3;
								return '&';
							}
						}
						else if (*src == 'p')
						{
							if
							(
								end-src >= 4 &&
								src[1] == 'o' &&
								src[2] == 's' &&
								src[3] == ';'
							)
							{
								string = src + 4;
								return '\'';
							}
						}
						break;

					case 'l':

						if
						(
							src[0] == 't' &&
							src[1] == ';'
						)
						{
							string = src + 2;
							return '<';
						}
						break;

					case 'g':

						if
						(
							src[0] == 't' &&
							src[1] == ';'
						)
						{
							string = src + 2;
							return '>';
						}
						break;

					case 'q':

						if
						(
							end-src >= 4 &&
							src[0] == 'u' &&
							src[1] == 'o' &&
							src[2] == 't' &&
							src[3] == ';'
						)
						{
							string = src + 4;
							return '\"';
						}
						break;
				}
			}

			return '\0';
		}

		dword Xml::Node::NumAttributes() const
		{
			dword n = 0;

			if (node)
			{
				for (const BaseNode::Attribute* attribute = node->attribute; attribute; attribute = attribute->next)
					++n;
			}

			return n;
		}

		dword Xml::Node::NumChildren(wcstring type) const
		{
			dword n = 0;

			if (node)
			{
				for (const BaseNode* next = node->child; next; next = next->sibling)
					n += (!type || !*type || IsEqual( next->type, type ));
			}

			return n;
		}

		Xml::Attribute Xml::Node::GetAttribute(dword i) const
		{
			BaseNode::Attribute* next = NULL;

			if (node)
			{
				for (next = node->attribute; i && next; --i)
					next = next->next;
			}

			return next;
		}

		Xml::Attribute Xml::Node::GetAttribute(wcstring type) const
		{
			if (node)
			{
				if (!type)
					type = L"";

				for (BaseNode::Attribute* next = node->attribute; next; next = next->next)
				{
					if (IsEqual( next->type, type ))
						return next;
				}
			}

			return NULL;
		}

		Xml::Node Xml::Node::GetChild(dword i) const
		{
			BaseNode* next = node;

			if (next)
			{
				for (next = node->child; i && next; --i)
					next = next->sibling;
			}

			return next;
		}

		Xml::Node Xml::Node::GetChild(wcstring type) const
		{
			if (node)
			{
				if (!type)
					type = L"";

				for (BaseNode* next = node->child; next; next = next->sibling)
				{
					if (IsEqual( next->type, type ))
						return next;
				}
			}

			return NULL;
		}

		long Xml::Node::GetSignedValue(uint base) const
		{
			return ToSigned( GetValue(), base, NULL );
		}

		long Xml::Node::GetSignedValue(wcstring& end,uint base) const
		{
			return ToSigned( GetValue(), base, &end );
		}

		ulong Xml::Node::GetUnsignedValue(uint base) const
		{
			return ToUnsigned( GetValue(), base, NULL );
		}

		ulong Xml::Node::GetUnsignedValue(wcstring& end,uint base) const
		{
			return ToUnsigned( GetValue(), base, &end );
		}

		Xml::BaseNode* Xml::Node::Add(wcstring type,wcstring value,BaseNode** next) const
		{
			while (*next)
				next = &(*next)->sibling;

			*next = new BaseNode( type, type + std::wcslen(type), BaseNode::OUT );

			if (value && *value)
				(*next)->SetValue( value, value + std::wcslen(value), BaseNode::OUT );

			return *next;
		}

		Xml::Attribute Xml::Node::AddAttribute(wcstring type,wcstring value)
		{
			if (type && *type && node)
			{
				BaseNode::Attribute** next = &node->attribute;

				while (*next)
					next = &(*next)->next;

				*next = new BaseNode::Attribute
				(
					type,
					type + std::wcslen(type),
					value ? value : L"",
					value ? value + std::wcslen(value) : NULL,
					BaseNode::OUT
				);

				return *next;
			}

			return NULL;
		}

		Xml::Node Xml::Node::AddChild(wcstring type,wcstring value)
		{
			return (type && *type && node) ? Add( type, value, &node->child ) : NULL;
		}

		Xml::Node Xml::Node::AddSibling(wcstring type,wcstring value)
		{
			return (type && *type && node) ? Add( type, value, &node->sibling ) : NULL;
		}

		long Xml::Attribute::GetSignedValue(uint base) const
		{
			return ToSigned( GetValue(), base, NULL );
		}

		long Xml::Attribute::GetSignedValue(wcstring& end,uint base) const
		{
			return ToSigned( GetValue(), base, &end );
		}

		ulong Xml::Attribute::GetUnsignedValue(uint base) const
		{
			return ToUnsigned( GetValue(), base, NULL );
		}

		ulong Xml::Attribute::GetUnsignedValue(wcstring& end,uint base) const
		{
			return ToUnsigned( GetValue(), base, &end );
		}
	}
}
