/*
 * Copyright(c) 2016, OpenAV
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior
 *       written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL OPENAV BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OPENAV_AVTK_LIST_HXX
#define OPENAV_AVTK_LIST_HXX

#include "group.hxx"

#include <string>
#include <vector>

namespace Avtk
{

/** List
 * A widget that wraps a group, and displays a list of strings as a neat choice
 * list box. ListItem widgets are placed inside, but other widgets can be added
 * too: a ListItem is a normal AVTK Widget.
 */
class List : public Group
{
public:
	List( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
	virtual ~List() {}
	virtual void draw( cairo_t* cr );

	void addItem( std::string newItem );

	/// integer input for which item to highlight
	void value( float v );
	/// returns the integer value of the clicked item
	float value()
	{
		return lastClickedItem;
	}

	void show( std::vector< std::string > data );

	virtual void clear();

	std::string selectedString();

protected:
	std::vector< std::string > items;

	int lastClickedItem;
	virtual void valueCB( Widget* w );
};

};

#endif // OPENAV_AVTK_LIST_HXX
