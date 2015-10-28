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
var outcomp;
// from http://stackoverflow.com/a/1219983/841108
function htmlEncode(value){
  //create a in-memory div, set it's inner text(which jQuery automatically encodes)
  //then grab the encoded contents back out.  The div never exists on the page.
  return $('<div/>').text(value).html();
}

function htmlDecode(value){
  return $('<div/>').html(value).text();
}

function got_hackc_click(ev)
{
    var srcprologue = prologuecodmir.getValue();
    var srcinitial = initialcodmir.getValue();
    console.debug ("got_hackc_click ev=", ev,
		   " srcprologue=", srcprologue,
		   " srcinitial=", srcinitial);
    $.ajax(
	{
	    url: "/mom_hackc_code",
	    method: "POST",
	    data: {
		do_hackc: "ajaxhack",
		prologuetxt: srcprologue,
		initialtxt: srcinitial
	    },
	    dataType: "JSON",
	    success: function (answer) {
		console.debug ("hackc answer=", answer);
		if (answer.compilation) {
		    outcomp.html("<h3>compilation <tt>" + answer.hackitem + "</tt> success</h3>" 
				 +"<pre class='compilemsg'>" + htmlEncode(answer.compileroutput) + "</pre>");
		}
		else {
		    outcomp.html("<h3>compilation <tt>" + answer.hackitem + "</tt> failure</h3>" 
				 +"<pre class='compilemsg'>" + htmlEncode(answer.compileroutput) + "</pre>");
		}
	    }
	});
    
};				// end got_hackc_click


$(document).ready(function(){
    prologuetxa = $("#prologuetxa_id");
    initialtxa = $("#initialtxa_id");
    hacksubmit = $("#hackc_id");
    outcomp = $("#outcomp_id");
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
    prologuecodmir.setSize({height:"25%"});
    initialcodmir.setSize({height:"55%"});
    hacksubmit.click(got_hackc_click);
});				// end document ready function
