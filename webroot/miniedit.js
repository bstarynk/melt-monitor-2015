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

console.log ("miniedit.js $webhost=", $webhost);

$(document).ready(function(){
    console.log ("miniedit document read start");
    $websocket = new WebSocket('ws://' + $webhost + "/miniedit_websocket");
    console.log ("miniedit $websocket=", $websocket);
}
