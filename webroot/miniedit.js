// file miniedit.js - Javascript & Jquery for miniedit.html.

/**   Copyright (C)  2016 Basile Starynkevitch, later FSF
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

var $webhost = window.location.host;
var $websocket;
var $handlermapwebsock = new Object;
var $navbar;
var $progstatusp;
var $tempstatusp;
var $contentdiv;
var $dumpexitbut;

function mom_register_websock(opnam,fun) {
    console.debug ("mom_register_websock opnam=", opnam, " fun=", fun);
    if (fun)
	$handlermapwebsock[opnam] = fun;
    else
	delete $handlermapwebsock[opnam];
}

function mom_content_updated() {
    console.log ("mom_content_updated $contentdiv=", $contentdiv);
    $($contentdiv).find(".mom_minieditvalue_cl")
	//.input(function(ev) {console.log ("minieditvalue input ev=", ev);})
	.keypress(function(ev){
	    console.log("mineditvalue keypress $(this)=", $(this)," ev=", ev);
	    return false;
	})
	.each(function (ix,el){console.log("minieditvalue ix=", ix, " el=", el);});
}

console.log ("miniedit.js $webhost=", $webhost);

$(document).ready(function(){
    console.log ("miniedit document read start $webhost=", $webhost);
    $navbar = $('#mom_minieditnavbar_id');
    $progstatusp = $('#mom_minieditprogramstatusp_id');
    $tempstatusp = $('#mom_miniedittempstatusp_id');
    $contentdiv = $('#mom_minieditcontentdiv_id');
    $dumpexitbut = $('#mom_minieditdumpexit_id');
    console.log ("miniedit readying $navbar=", $navbar,
		 " $progstatusp=", $progstatusp,
		 " $tempstatusp=", $tempstatusp,
		 " $contentdiv=", $contentdiv,
		 " $dumpexitbut=", $dumpexitbut,
		 " $websocket=", $websocket);
    if (!$websocket) {
	console.log("miniedit getting websocket");
	$websocket = new WebSocket('ws://' + $webhost + "/mom_websocket");
    }
    console.log ("miniedit $websocket=", $websocket);
    $websocket.onopen = function () {
	console.log ("miniedit opened $websocket=", $websocket);
    };
    $websocket.onmessage = function(evt) {
	console.log ("miniedit $websocket=", $websocket,
		     " message evt=", evt);
	var jp = JSON.parse(evt.data);
	var jop = null;
	var jfun = null;
	console.log ("miniedit websockmessage jp=", jp);
	if ("$momevap" in jp) {
	    var apst = jp["$momevap"];
	    console.log ("miniedit websockmessage apst=", apst);
	    var evfu = eval(apst);
	    console.log ("miniedit websockmessage evfu=", evu);
	    if (evfu)
		evfu(jp, evt);
	}
	else if ("$op" in jp && typeof(jop=jp["$op"])==="string") {
	    console.log ("miniedit websockmessage jop=", jop);
	    if (jop in $handlermapwebsock) {
		jfun =  $handlermapwebsock[jop];
		console.log ("miniedit websockmessage jfun=", jfun);
		jfun(jp, evt);
		console.log ("miniedit websockmessage done jfun=", jfun, " on jp=", jp);
	    }
	    else console.warning("miniedit websockmessage bad jop=", jop);
	}
    };
    $websocket.onclose = function() {
	console.log ("miniedit closed $websocket=", $websocket);
	$websocket = null;
    };
    console.log ("miniedit $dumpexitbut=", $dumpexitbut);
    $dumpexitbut.click(function(ev) {
	console.log("miniedit $dumpexitbut=", $dumpexitbut, " cliked ev=", ev);
	$.ajax({url: "/miniedit_dumpexit",
		method: "POST",
		data: {},
		dataType: "json",
		success: function(data, stat, jh) {
		    console.log("miniedit dumpexit ajaxok data=", data);
		    $tempstatusp.html("<b>dumped, exiting</b> at <i>"+data.now+"</i>"
				     + "; <small>("+data.elapsedreal.toPrecision(3)+ " real seconds,"
				     + " "+data.processcpu.toPrecision(3)+ " cpu seconds)</small>"
				    );
		},
		error: function(jq, stat, err) {
		    console.log("miniedit dumpedit ajaxerror jq=", jq, " stat=", stat, " err=", err);
		}
	       });			       
    });
    console.log("miniedit $contentdiv=", $contentdiv);
    $contentdiv.on('input', function(ev) {
	// http://stackoverflow.com/a/26353788/841108
	var fo = window.getSelection().getRangeAt(0);
	var pn = fo.commonAncestorContainer.parentNode;
	var wi = ev.which;
	console.log("miniedit input $contentdiv=", $contentdiv, " ev=", ev,
		    " $(this)=", $(this), " fo=", fo, " pn=", pn, " wi=", wi);
    });
    console.log("miniedit $contentdiv=", $contentdiv);
    $($contentdiv).keypress(function(ev) {
	var fo = window.getSelection().getRangeAt(0);
	var pn = fo.commonAncestorContainer.parentNode;
	var wi = ev.which;
	console.log("miniedit keypress $contentdiv=", $contentdiv, " ev=", ev,
		    " $(this)=", $(this), " fo=", fo, " pn=", pn, " wi=", wi);
    });
    console.log("miniedit $contentdiv=", $contentdiv);
    $($contentdiv).keyup(function(ev) {
	var fo = window.getSelection().getRangeAt(0);
	var pn = fo.commonAncestorContainer.parentNode;
	var wi = ev.which;
	console.log("miniedit keyup $contentdiv=", $contentdiv, " ev=", ev,
		    " $(this)=", $(this), " fo=", fo, " pn=", pn, " wi=", wi);
    });
    console.log("miniedit $contentdiv=", $contentdiv);
    $($contentdiv).keydown(function(ev) {
	var fo = window.getSelection().getRangeAt(0);
	var pn = fo.commonAncestorContainer.parentNode;
	var wi = ev.which;
	console.log("miniedit keydown $contentdiv=", $contentdiv, " ev=", ev,
		    " $(this)=", $(this), " fo=", fo, " pn=", pn, " wi=", wi);
    });
    $.ajax
    ({url: "/miniedit_startpage",
      method: "POST",
      data: {},
      dataType: "json",
      success: function (data, stat, jh) {
          console.log("miniedit_startpage success data=", data, " stat=", stat, " jh=", jh);
	  var ps = data.progstatus;
	  var hc = data.contenthtml;
	  var dc = data.docontent;
	  console.log("miniedit_startpage ps=", ps, "\n.. hc=", hc);
	  $progstatusp.html(ps);
	  $contentdiv.html(hc);
	  console.log("miniedit_startpage dc=", dc);
	  if (dc) eval(dc);
      },
      error: function (jq, stat, err) {
          console.log("miniedit_startpage error jq=", jq, " stat=", stat, " err=", err);
      }
     });
});
