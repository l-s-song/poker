var script = document.createElement("script");
script.setAttribute("src", "https://ajax.googleapis.com/ajax/libs/jquery/1.6.4/jquery.min.js");
script.addEventListener('load', function() {
  var script = document.createElement("script");
  document.body.appendChild(script);
}, false);
document.body.appendChild(script);

function test() {
  $.get("/api/login/10", console.log)
  for(let i = 0; i < 20; i++){
    $.post("/api/queue", '{"type": "nlhe", "format": "ring", "table_size": 6, "big_blind":  2}',
      console.log)
  }
  get_games();
}

function act(table_id, action, bet_size) {
  $.post("/api/act", '{"table_id": "' + table_id + '", "action": "' + action + '", "bet_size": ' + bet_size + '}',
  console.log)
}

function get_games() {
  $.get("/api/games", console.log)
}

function checkitdown(table_id, num_players) {
  for(let i = 0; i < num_players - 1; i++) {
    act(table_id, "call", 2);
  }
  act(table_id, "check", 2);
  for(let i = 0; i < num_players*3; i++) {
    act(table_id, "check", 0);
  }
}
