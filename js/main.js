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

function populateRows() {
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

function loadGames(){
  //returns data of all games
  xhr = new XMLHttpRequest();
  xhr.open('GET', 'http://localhost:8080/api/games');
  xhr.onload = function() {
    if (xhr.status === 200) {
      //console.log("Response: " + xhr.responseText);
      gamelist = JSON.parse(xhr.responseText);
      //console.log(gamelist);
    } else {
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
    populateRows();
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

  let sizes = {
    "headsup": false,
    "sixmax": false,
    "fullring": false,
  };

  if (document.getElementById('filter-size-headsup').checked) {
    sizes.headsup = true;
  }
  if (document.getElementById('filter-size-6max').checked) {
    sizes.sixmax = true;
  }
  if (document.getElementById('filter-size-fullring').checked) {
    sizes.fullring = true;
  }

  return {
    games,
    format,
    sizes,
  };
}

function populateTable(table){
  //type(table) = array {Game, Name, Players, Table Size, Blinds}
  var newtable = document.createElement("TABLE");
  let thead = newtable.createTHead();
  let row = thead.insertRow();
  const data = ["Game", "Name", "Players", "Table Size", "Blinds"];
  for(let key of data){
    let td = document.createElement("td");
    let textNode = document.createTextNode(key);
    td.appendChild(textNode);
    row.appendChild(td);
  }
  for(let element of table){
    elem = [
      element.type,
      "NYC",
      "5",
      element.table_size,
      element.big_blind + "/" + (element.big_blind/2)
      //\u00A2
    ];
    let row = newtable.insertRow();
    for(key of elem){
      let cell = row.insertCell();
      let textNode = document.createTextNode(key);
      cell.appendChild(textNode);
    }
  }
  document.getElementById("poker-tables").innerHTML =
    newtable.innerHTML;
}

function updateTable(){
  loadGames();
  //console.log(gamelist);
  populateTable(gamelist);
}

populateRows();
//populateTable([{type:"NLHE", city:"Anchorage", active_players:3, table_size:6, bigblind:200},
//  {type:"PLO", city:"Seattle", active_players:2, table_size:6, bigblind:100}]);
//i = setInterval(updateTable, 300);
setInterval(updateTable, 300);
