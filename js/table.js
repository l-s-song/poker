const URL = window.location.pathname;
const tableID = /^\/table\/([0-9]+)$/g.exec(URL)[1];
let canvas = document.getElementById("table-canvas");
let ctx = canvas.getContext("2d");
let table = document.getElementById("table");
let playerButton = document.getElementById("player-button");

function loadTable(callback){
  //returns data of all games
  xhr = new XMLHttpRequest();
  xhr.open('GET', '/api/table/' + tableID);
  xhr.onload = function() {
    if (xhr.status === 200) {
      tableData = JSON.parse(xhr.responseText);
      //console.log(gamelist);
      callback(tableData);
    } else {
      console.error(xhr.status, xhr.responseText);
      callback(tableData);
    }
  }
  xhr.send();
}

const show_debug_rects = true;

const pixel_inner_table_width = 1216;
const pixel_inner_table_height = 634;
const pixel_inner_table_margin = 47;
const pixel_outer_table_width = 2235;
const pixel_outer_table_height = 1050;

const pixel_player_width = 514 - 49;
const pixel_player_height = 213 - 49;
const pixel_player_margin = 49;

const outer_table_width_to_height = pixel_outer_table_width/pixel_outer_table_height;
const inner_table_width_to_height = pixel_inner_table_width/pixel_inner_table_height;
const inner_table_margin_to_width = pixel_inner_table_margin/pixel_inner_table_width;
const inner_table_margin_to_height = pixel_inner_table_margin/pixel_inner_table_height;
const inner_table_width_to_outer_table_width = pixel_inner_table_width/pixel_outer_table_width;
const inner_table_height_to_outer_table_height = pixel_inner_table_height/pixel_outer_table_height;

const player_width_to_height = pixel_player_width/pixel_player_height;
const player_margin_to_width = pixel_player_margin/pixel_player_width;
const player_margin_to_height = pixel_player_margin/pixel_player_height;


function drawDebugRect(x, y, width, height) {
  if (show_debug_rects) {
    const oldStrokeStyle = ctx.strokeStyle;
    ctx.strokeStyle = "red";
    ctx.rect(x + 1, y + 1, width - 3, height - 3);
    ctx.stroke();
    ctx.strokeStyle = oldStrokeStyle;
  }
}

function drawInnerTable(x, y, width, height){
  drawDebugRect(x, y, width, height);
  const horizontal_margin = inner_table_margin_to_width * width;
  x -= horizontal_margin;
  width += 2 * horizontal_margin;
  const vertical_margin = inner_table_margin_to_height * height;
  y -= vertical_margin;
  height += 2 * vertical_margin;
  ctx.drawImage(table, x, y, width, height);
}

function drawPlayerButton(x, y, width, height){
  drawDebugRect(x, y, width, height);
  const horizontal_margin = player_margin_to_width * width;
  x -= horizontal_margin;
  width += 2 * horizontal_margin;
  const vertical_margin = player_width_to_height * height;
  y -= vertical_margin;
  height += 2 * vertical_margin;
  ctx.drawImage(playerButton, x, y, width, height);
}

function drawOuterTable(x, y, width, height){
  drawDebugRect(x, y, width, height);
  let tableCenter = {x: x + width/2, y: y + height/2};
  const inner_table_width = inner_table_width_to_outer_table_width*width;
  const inner_table_height = inner_table_height_to_outer_table_height*height;
  drawInnerTable(
    tableCenter.x - inner_table_width / 2,
    tableCenter.y - inner_table_height / 2,
    inner_table_width,
    inner_table_height
  );

}

function redraw(){
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    drawDebugRect(0, 0, canvas.width, canvas.height);
    console.log("hello");

    const opt1 = [outer_table_width_to_height * canvas.height, canvas.height];
    const opt2 = [canvas.width, canvas.width / outer_table_width_to_height];

    if (opt1[0] < opt2[0]) {
      drawOuterTable(0, 0, opt1[0], opt1[1]);
    } else {
      drawOuterTable(0, 0, opt2[0], opt2[1]);
    }
}

window.onload = function() {
  loadTable(function (tableData) {
    console.log(tableData);
  });
  redraw();
  window.addEventListener('resize', redraw);
}
