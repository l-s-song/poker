function createSnapjoinButton(snapjoinData) {
  ret = '';
  ret += '<div id="' + snapjoinData.id + '" class="snapjoin-button">';
  ret += '\t<button>' + snapjoinData.blinds + '</button>';
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

function populateSnapjoinRows() {
  const snapjoinData = getSnapjoinData(getSettings());
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
      callback(gamelist);
    } else {
      console.log(xhr.status, xhr.responseText);
      alert('loadGames failed');
    }
  }
  xhr.send();
}

function getSnapjoinData(settings) {
  const blinds = [
                    '$1/$2','$2/$4','$5/$10','$10/$20','$20/$40',
                    '2&cent;/4&cent;', '5&cent;/10&cent;', '10&cent;/20&cent;', '20&cent;/40&cent;', '50&cent;/$1'
                 ];
  const ret = [];
  for(let i = 1; i <= 10; i++) {
    const buttonData = {};
    buttonData.id = 'snapjoin-button-' + i;
    buttonData.blinds = blinds[i-1];
    buttonData.playersNeeded = Math.floor(Math.random() * 3 + 4);
    buttonData.playersWaiting = Math.floor(Math.random() * buttonData.playersNeeded);
    ret.push(buttonData);
  }
  return ret;
}

function updateSettings() {
  const settings = getSettings();
  if (settings.format.tournament == true) {
    document.getElementById("snapjoin-section").classList.add("hidden");
  } else {
    document.getElementById("snapjoin-section").classList.remove("hidden");
    populateSnapjoinRows();
  }
}

function getSettings() {
  games = {
    nlhe: true,
    plo: true,
  };
  if (document.getElementById('filter-game-nlhe').checked) {
    games.plo = false;
  } else if (document.getElementById('filter-game-plo').checked) {
    games.nlhe = false;
  }

  let format = {
    ring: false,
    sitngo: false,
    tournament: false,
  }

  if (document.getElementById('filter-format-ring').checked) {
    format.ring = true;
  } else if (document.getElementById('filter-format-sitngo').checked) {
    format.sitngo = true;
  } else {
    format.tournament = true;
  }

  let tableSize;

  if (document.getElementById('filter-size-headsup').checked) {
    tableSize = 2;
  }
  if (document.getElementById('filter-size-6max').checked) {
    tableSize = 6;
  }
  if (document.getElementById('filter-size-fullring').checked) {
    tableSize = 9;
  }

  return {
    games,
    format,
    tableSize,
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

function updateGames(){
  loadGames(function(games) {
    populateGamesTable(games);

  });
}

populateSnapjoinRows();
//populateTable([{type:"NLHE", city:"Anchorage", active_players:3, table_size:6, bigblind:200},
//  {type:"PLO", city:"Seattle", active_players:2, table_size:6, bigblind:100}]);
//i = setInterval(updateTable, 300);
setInterval(updateGames, 300);
