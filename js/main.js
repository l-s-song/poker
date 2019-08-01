function createSnapjoinButton(snapjoinData) {
  ret = '';
  ret += '<div id="' + snapjoinData.id + '" class="snapjoin-button">';
  ret += '\t<button onclick="addToQueue()">' + snapjoinData.blinds + '</button>';
  ret += '\t<div class="snapjoin-loading">';
  for(let i = 0; i < snapjoinData.playersNeeded; i++) {
    ret += '\t\t<div class="snapjoin-loading-cell' +
      (i < snapjoinData.playersWaiting ? ' active' : '')
      + '"></div>';
  }
  ret += '\t</div>';
  ret += '</div>';

  return ret;
}

/*
<div class="snapjoin-button">
  <button>$1/$2</button>
  <div class="snapjoin-loading">
    <div class="snapjoin-loading-cell active"></div>
    <div class="snapjoin-loading-cell active"></div>
    <div class="snapjoin-loading-cell"></div>
    <div class="snapjoin-loading-cell"></div>
    <div class="snapjoin-loading-cell"></div>
    <div class="snapjoin-loading-cell"></div>
  </div>
</div>
*/

function populateSnapjoin(queueData, gameList) {
  const snapjoinData = getSnapjoinData(queueData, gameList, getSettings());
  let row1 = document.getElementById('snapjoin-buttons-1');
  row1.innerHTML = "";
  for (let i = 0; i < 5; i++) {
    row1.innerHTML += createSnapjoinButton(snapjoinData[i]);
  }
  let row2 = document.getElementById('snapjoin-buttons-2');
  row2.innerHTML = "";
  for (let i = 5; i < 10; i++) {
    row2.innerHTML += createSnapjoinButton(snapjoinData[i]);
  }
}

gamelist = [];

function loadGames(callback){
  //returns data of all games
  xhr = new XMLHttpRequest();
  xhr.open('GET', 'api/games');
  xhr.onload = function() {
    if (xhr.status === 200) {
      gamelist = JSON.parse(xhr.responseText);
      //console.log(gamelist);
      callback(gamelist);
    } else {
      console.log(xhr.status, xhr.responseText);
      alert('loadGames failed');
    }
  }
  xhr.send();
}

function loadQueue(callback){
  xhr = new XMLHttpRequest();
  xhr.open('GET', 'api/queue');
  xhr.onload = function() {
    if (xhr.status === 200) {
      queue = JSON.parse(xhr.responseText);
      callback(queue);
    } else {
      console.log(xhr.status, xhr.responseText);
      alert('loadGames failed');
    }
  }
  xhr.send();
}

function getSnapjoinData(queueData, gameList, settings) {
  //returns [{id, blinds, playersNeeded, playersWaiting}]
  const bigBlinds = [200, 400, 1000, 2000, 4000, 4, 10, 20, 40, 100];
  const blindsformat = [
    '$1/$2','$2/$4','$5/$10','$10/$20','$20/$40',
    '2&cent;/4&cent;', '5&cent;/10&cent;', '10&cent;/20&cent;', '20&cent;/40&cent;', '50&cent;/$1'
  ];
  const ret = [];
  let playersNeeded = Array(10).fill(-1);
  let playersWaiting = Array(10).fill(-1);
  if (!window.firstblah)
    console.log(settings);
  //get game that can be joined
  for(let i = 0; i < gameList.length; i++){
    for(let j = 0; j < bigBlinds.length; j++){
      if (!window.firstblah)
          console.log(gameList);
      if(playersNeeded[j] == -1
      && gameList[i].big_blind == bigBlinds[j]
      && settings.format == gameList[i].format
      && (settings.type == "any" || settings.type == gameList[i].type)
      && settings.sizes["" + gameList[i].table_size]
      && gameList[i].num_players < gameList[i].table_size
      ){
        if (!window.firstblah)
          console.log(gameList[i]);
        playersNeeded[j] = Math.floor(gameList[i].table_size/2) + 1;
        playersWaiting[j] = gameList[i].num_players;
      }
    }
  }
  //look through queue for players wanting the same settings
  for(let i = 0; i < playersNeeded.length; i++){
    for (let j = 0; j < queueData.length; j++){
      if((settings.type == "any"
       || settings.type == queueData[j].type)
       && settings.format == queueData[j].format
       && settings.sizes[queueData[j].table_size]
       && bigBlinds[i] == queueData[j].big_blind
     ){
       queuePlayersNeeded = Math.floor(queueData[j].table_size/2) + 1;
       if (playersNeeded[i] == -1){
         playersNeeded[i] = queuePlayersNeeded;
         playersWaiting[i] = queueData[j].num_players;
       } else {
         if ((queuePlayersNeeded - queueData[j].num_players)
           < (playersNeeded[i] - playersWaiting[i])
         ){
           playersNeeded[i] = queuePlayersNeeded;
           playersWaiting[i] = queueData[j].num_players;
         }
       }
     }
    }
  }
  //if no one is in the queue and no tables are available
  for(let i = 0; i < playersNeeded.length; i++){
    if(playersNeeded[i] == -1){
      if (settings.sizes["2"]){
        playersNeeded[i] = 1;
      } else if (settings.sizes["6"]){
        playersNeeded[i] = 4;
      } else {
        playersNeeded[i] = 5;
      }
      playersWaiting[i] = 0;
    }
  }

  for(let i = 1; i <= 10; i++) {
    const buttonData = {};
    buttonData.id = 'snapjoin-button-' + i;
    buttonData.blinds = blindsformat[i-1];
    buttonData.playersNeeded = playersNeeded[i-1];
    buttonData.playersWaiting = playersWaiting[i-1];
    if (!window.blah) {
      console.log(buttonData);
      window.blah = true;
    }
    ret.push(buttonData);
  }
  window.firstblah = true;
  return ret;
}

function updateSettings() {
  const settings = getSettings();
  if (settings.format.tournament == true) {
    document.getElementById("snapjoin-section").classList.add("hidden");
  } else {
    document.getElementById("snapjoin-section").classList.remove("hidden");
  }
  // TODO fix game table and snapjoin buttons
}

function getSettings() {
  let type = "";
  const isplo = document.getElementById('filter-game-plo').checked;
  const isnlhe = document.getElementById('filter-game-nlhe').checked;
  if (isplo && isnlhe){
    type = "any";
  } else if (isplo){
    type = "plo";
  } else {
    type = "nlhe";
  }

  let format = "";
  if (document.getElementById('filter-format-ring').checked) {
    format = "ring";
  } else if (document.getElementById('filter-format-sitngo').checked) {
    format = "sitngo";
  } else {
    format = "tournament";
  }

  let sizes = {
    "2": false,
    "6": false,
    "9": false,
  };

  if (document.getElementById('filter-size-headsup').checked) {
    sizes["2"] = true;
  }
  if (document.getElementById('filter-size-6max').checked) {
    sizes["6"] = true;
  }
  if (document.getElementById('filter-size-fullring').checked) {
    sizes["9"] = true;
  }

  return {
    type,
    format,
    sizes,
  };
}

function populateGamesTable(games){
  //type(table) = array {Game, Name, Players, Table Size, Blinds}
  let table = document.createElement("TABLE");
  let thead = table.createTHead();
  let row = thead.insertRow();
  const data = ["Game", "Name", "Players", "Table Size", "Blinds"];
  for(let key of data){
    let td = document.createElement("td");
    let textNode = document.createTextNode(key);
    td.appendChild(textNode);
    row.appendChild(td);
  }
  for(let element of games){
    elem = [
      element.type,
      "NYC",
      element.num_players,
      element.table_size,
      (element.big_blind/2) + "/" + element.big_blind
    ];
    let row = table.insertRow();
    for(key of elem){
      let cell = row.insertCell();
      let textNode = document.createTextNode(key);
      cell.appendChild(textNode);
    }
  }
  document.getElementById("poker-tables").innerHTML =
    table.innerHTML;
}

function updateTable(){
  loadGames(function(games) {
    populateTable(games);
    loadQueue(function(queue) {
      populateSnapjoin(queue, games);
      setTimeout(updateTable, 100);
    })
  });
}

//populateTable([{type:"NLHE", city:"Anchorage", active_players:3, table_size:6, bigblind:200},
//  {type:"PLO", city:"Seattle", active_players:2, table_size:6, bigblind:100}]);
//i = setInterval(updateTable, 300);
updateTable();
