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

#ifndef OPENAV_AVTK_FILE_BROWSER_HXX
#define OPENAV_AVTK_FILE_BROWSER_HXX

#include "group.hxx"

namespace Avtk
{

class Box;

/** FileBrowser
 * A generic file browsing / selection tool, that provides easy navigation of
 * the filesystem. Armed with hotkeys and shortcuts for power users, the user
 * experience is kept as simple as possible for lightning fast navigation.
 *
 * This class provides more callbacks than an Avtk::Widget.
 */
class FileBrowser : public Group
{
public:
	FileBrowser( Avtk::UI* ui, int x, int y, int w, int h, std::string label);
	virtual ~FileBrowser() {}

	virtual void draw( cairo_t* cr );

	/// a callback that is called when the user *changes* the selection. This is
	/// not the "OK" clicked callback: this is called once per selected file.
	/// use case: Auditioning audio samples : play file on selectionChanged()
	void (*selectionChanged)(Avtk::Widget* w, void* userdata);
	void*  selectionChangedUD;

private:
	Box* header;

};

};

#endif // OPENAV_AVTK_FILE_BROWSER_HXX
