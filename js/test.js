var script = document.createElement("script");
script.setAttribute("src", "https://ajax.googleapis.com/ajax/libs/jquery/1.6.4/jquery.min.js");
script.addEventListener('load', function() {
  var script = document.createElement("script");
  document.body.appendChild(script);
}, false);
document.body.appendChild(script);

function test() {
  for(let i = 0; i < 20; i++){
    $.get("/api/login/" + Math.floor(Math.random()*1000000), console.log)
    $.post("/api/queue", '{"type": "nlhe", "format": "ring", "table_size": 6, "big_blind":  200}',
      console.log)
  }
  get_games();
}

function act(table_id, action, bet_size, callback) {
  get_table(table_id, function(table){
    let current_turn = table.current_turn;
    let current_player = table.player_ids[current_turn];
    $.get("/api/login/" + current_player, function(){
      $.post("/api/act", '{"table_id": "' + table_id + '", "action": "' + action + '", "bet_size": ' + bet_size + '}', function(data, status) {
        console.log(data, status)
        callback()
      })
    });
  })
}

function get_games() {
  $.get("/api/games", console.log)
}

function get_table(table_id, callback){
  let table;
  $.get("/api/table/" + table_id, function(data, status) {
    // console.log(data);
    table = JSON.parse(data);
    callback(table);
  })

}

function checkitdown(table_id, num_players) {
  let do_it;
  do_it = function(i) {
    if( i < num_players - 1) {
      act(table_id, "call", 2, function() {
        do_it(i + 1)
      })
    } else if (i == num_players - 1) {
      act(table_id, "check", 2, function() {
        do_it(i + 1)
      })
    } else if (i < num_players * 4) {
      act(table_id, "check", 0, function() {
        do_it(i+1)
      })
    }
  }
  do_it(0)
}
