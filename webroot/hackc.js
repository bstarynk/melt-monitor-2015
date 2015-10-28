// file hackc.js - Javascript & Jquery for hackc.html.

/**   Copyright (C)  2015 Basile Starynkevitch, later FSF
      MONIMELT is a monitor for MELT - see http://gcc-melt.org/
      This file is part of GCC.
      
      GCC is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 3, or (at your option)
      any later version.
      
      GCC is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
      You should have received a copy of the GNU General Public License
      along with GCC; see the file COPYING3.   If not see
      <http://www.gnu.org/licenses/>.
**/

var prologuetxa;
var initialtxa;
var prologuecodmir;
var initialcodmir;
var hackcsubmit;

function got_hackc_click(ev)
{
    console.debug ("got_hackc_click ev=", ev);
    $.ajax(
	{
	    url: "/mom_hackc_code",
	    method: "POST",
	    data: {
		do_hackc: "ajaxhack",
		prologuetxt: prologuetxa.text(),
		initialtxt: initialtxa.text()
	    },
	    dataType: "JSON",
	    success: function (answer) {
		console.debug ("hackc answer=", answer);
	    }
	});
    
};				// end got_hackc_click


$(document).ready(function(){
    prologuetxa = $("#prologuetxa_id");
    initialtxa = $("#initialtxa_id");
    hacksubmit = $("#hackc_id");
    console.debug ("documready prologuetxa=", prologuetxa,
		   " initialtxa=", initialtxa,
		   " hacksubmit=", hacksubmit);
    prologuecodmir = CodeMirror.fromTextArea(prologuetxa[0], {
	lineNumbers: true,
	theme: "default",
	mode: "text/x-csrc"
    });
    initialcodmir = CodeMirror.fromTextArea(initialtxa[0], {
	lineNumbers: true,
	theme: "neo",
	mode: "text/x-csrc"
    });
    hacksubmit.click(got_hackc_click);
});				// end document ready function
