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

#include "list.hxx"

#include "ui.hxx"
#include "theme.hxx"
#include "listitem.hxx"

#include <stdio.h>

using namespace Avtk;

List::List( Avtk::UI* ui, int x_, int y_, int w_, int h_, std::string label_) :
	Group( ui, x_, y_, w_, h_, label_ )
{
	mode( Group::WIDTH_EQUAL );
	valueMode( Group::VALUE_SINGLE_CHILD );
	lastClickedItem = -1;
}

void List::addItem( std::string newItem )
{
	items.push_back( newItem );
	add( new Avtk::ListItem( ui, 0, 0, 34, 14, newItem ) );
}

void List::show( std::vector< std::string > data )
{
	items.swap( data );

	for(int i = 0; i < items.size(); i++ ) {
		add( new Avtk::ListItem( ui, 0, 0, 11, 11, items.at(i) ) );
	}
}

void List::value( float v )
{
	int item = int(v);
	for( int i = 0; i < children.size(); i++ ) {
		bool v = false;
		if( item == i )
			v = true;

		children.at(i)->value( v );
	}
	lastClickedItem = item;
}

void List::clear()
{
	// free the widgets
	Group::clear();
	items.clear();
	// invalidate last item
	lastClickedItem = -1;
}

void List::draw( cairo_t* cr )
{
	if( Widget::visible() ) {
		cairo_save( cr );

		Group::draw( cr );

		cairo_restore( cr );
	}
}

std::string List::selectedString()
{
	if( lastClickedItem == -1 ||
	    lastClickedItem >= items.size() ) {
		return "";
	}
	return items.at( lastClickedItem );
}

void List::valueCB( Widget* w )
{
	// call the super valueCB, handles turning off other widgets
	Group::valueCB( w );
	lastClickedItem = w->groupItemNumber();

	// copy mouse event co-ords to List widget
	mouseButtonPressed_ = w->mouseButton();
	mousePressX = w->mouseX();
	mousePressY = w->mouseY();

	std::string tmp = selectedString();
	if( !tmp.size() ) {
		return;
	}

	printf("list: lastClickedItem# %i, string: %s\n", lastClickedItem, selectedString().c_str() );

	// send an event to UI as the list widget
	if( callback )
		callback( this, callbackUD );

	//Avtk::UI::staticWidgetValueCB( this, ui );
}
