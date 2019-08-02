function createSnapjoinButton(snapjoinData) {
  const blindsFormat = {
    200: '$1/$2',
    400: '$2/$4',
    1000: '$5/$10',
    2000: '$10/$20',
    4000: '$20/$40',
    4: '2&cent;/4&cent;',
    10: '5&cent;/10&cent;',
    20: '10&cent;/20&cent;',
    40: '20&cent;/40&cent;',
    100: '50&cent;/$1'
  };
  ret = '';
  ret += '<div id="' + snapjoinData.id + '" class="snapjoin-button">';
  ret += '\t<button onclick="addToQueue(' + snapjoinData.bigBlind + ')">' + blindsFormat[snapjoinData.bigBlind] + '</button>';
  ret += '\t<div id="' + snapjoinData.id + '-loading" class="snapjoin-loading">';
  for(let i = 0; i < snapjoinData.playersNeeded - 1; i++) {
    ret += '\t\t<div class="snapjoin-loading-cell' +
      (i < snapjoinData.playersWaiting ? ' active' : '')
      + '"></div>';
  }
  ret += '\t</div>';
  ret += '</div>';

  return ret;
}

function updateSnapjoinLoadingBar(id, playersWaiting, playersNeeded) {
  loaderHTML = '';
  for(let i = 0; i < playersNeeded - 1; i++) {
    loaderHTML += '\t\t<div class="snapjoin-loading-cell' +
      (i < playersWaiting ? ' active' : '')
      + '"></div>';
  }
  let loader = document.getElementById(id + '-loading');
  loader.innerHTML = loaderHTML;
}

function addToQueue(bigBlind){
  //upon click of button with bigBlind = big blind
  const settings = getSettings();
  xhr = new XMLHttpRequest();
  xhr.open('POST', 'api/queue');
  xhr.onload = function() {
    if (xhr.status === 200) {
    } else {
      console.error(xhr.status, xhr.responseText);
    }
  }
  xhr.send(JSON.stringify(
    {
      types: settings.types,
      format: settings.format,
      table_sizes: settings.table_sizes,
      big_blind: bigBlind
    }
  ));
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

function populateSnapjoin() {
  const snapjoinData = getSnapjoinData();
  let row = "";
  for (let i = 0; i < 5; i++) {
    row += createSnapjoinButton(snapjoinData[i]);
  }
  let row1 = document.getElementById('snapjoin-buttons-1');
  row1.innerHTML = row;
  row = "";
  for (let i = 5; i < 10; i++) {
    row += createSnapjoinButton(snapjoinData[i]);
  }
  let row2 = document.getElementById('snapjoin-buttons-2');
  row2.innerHTML = row;
}

function updateSnapjoin(queueData, gameList) {
  const snapjoinData = getSnapjoinData(queueData, gameList);
  for(let buttonData of snapjoinData) {
    updateSnapjoinLoadingBar(buttonData.id, buttonData.playersWaiting, buttonData.playersNeeded);
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
      console.error(xhr.status, xhr.responseText);
      callback();
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
      console.error(xhr.status, xhr.responseText);
      callback();
    }
  }
  xhr.send();
}

function getSnapjoinData(queueData, gameList) {
  const settings = getSettings();
  //returns [{id, blinds, playersNeeded, playersWaiting}]
  const bigBlinds = [200, 400, 1000, 2000, 4000, 4, 10, 20, 40, 100];
  const ret = [];
  let playersNeeded = Array(10).fill(-1);
  let playersWaiting = Array(10).fill(-1);

  if (queueData && gameList) {
    //get game that can be joined
    for(let i = 0; i < gameList.length; i++){
      for(let j = 0; j < bigBlinds.length; j++){
        if(playersNeeded[j] == -1
        && gameList[i].big_blind == bigBlinds[j]
        && settings.format == gameList[i].format
        && settings.types.includes(gameList[i].type)
        && settings.table_sizes.includes(gameList[i].table_size)
        && gameList[i].num_players < gameList[i].table_size
        ){
          playersNeeded[j] = Math.floor(gameList[i].table_size/2) + 1;
          playersWaiting[j] = gameList[i].num_players;
        }
      }
    }
    //look through queue for players wanting the same settings
    for(let i = 0; i < playersNeeded.length; i++){
      for (let j = 0; j < queueData.length; j++){
        if(  settings.types.includes(queueData[j].type)
          && settings.format == queueData[j].format
          && settings.table_sizes.includes(queueData[j].table_size)
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
  }

  for(let i = 1; i <= 10; i++) {
    const buttonData = {};
    buttonData.id = 'snapjoin-button-' + i;
    buttonData.bigBlind = bigBlinds[i-1];
    buttonData.playersNeeded = playersNeeded[i-1];
    buttonData.playersWaiting = playersWaiting[i-1];
    if (!window.blah) {
      console.log(buttonData);
      window.blah = true;
    }
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
  }
  populateSnapjoin();
  // TODO fix game table and snapjoin buttons
}

function getSettings() {
  let types = [];
  const isplo = document.getElementById('filter-game-plo').checked;
  const isnlhe = document.getElementById('filter-game-nlhe').checked;
  const isany = document.getElementById('filter-game-any').checked;
  if (isany){
    isplo = true;
    isnlhe = true;
  }
  if (isplo){
    types.push("plo");
  }
  if (isnlhe){
    types.push("nlhe");
  }

  let format = "";
  if (document.getElementById('filter-format-ring').checked) {
    format = "ring";
  } else if (document.getElementById('filter-format-sitngo').checked) {
    format = "sitngo";
  } else {
    format = "tournament";
  }

  // sorted smallest to largest
  let table_sizes = [];

  if (document.getElementById('filter-size-headsup').checked) {
    table_sizes.push(2);
  }
  if (document.getElementById('filter-size-6max').checked) {
    table_sizes.push(6);
  }
  if (document.getElementById('filter-size-fullring').checked) {
    table_sizes.push(9);
  }

  return {
    types,
    format,
    table_sizes,
  };
}

function populateGamesTable(games){
  settings = getSettings();
  //type(table) = array {Game, Name, Players, Table Size, Blinds}
  let table = document.createElement("TABLE");
  let thead = table.createTHead();
  let row = thead.insertRow();
  const data = ["Game", "Name", "Players", "Table Size", "Blinds"];
  for(let key of data){
    let th = document.createElement("th");
    let textNode = document.createTextNode(key);
    th.appendChild(textNode);
    row.appendChild(th);
  }
  for(let element of games){
    if (
         settings.types.includes(element.type)
      && element.format == settings.format
      && settings.table_sizes.includes(element.table_size)
    ){
      elem = [
        element.type,
        "NYC",
        element.num_players,
        element.table_size,
        (element.big_blind/2) + "/" + element.big_blind
      ];
      let row = table.insertRow();
      for(key of elem){
        let td = document.createElement("td");
        let textNode = document.createTextNode(key);
        td.appendChild(textNode);
        row.appendChild(td);
      }
    }
  }
  document.getElementById("poker-tables").innerHTML = table.innerHTML;
  filterTable();
  listActiveGames();
}

function updateTable(){
  if (window.stopUpdating) {
    return;
  }
  loadGames(function(games) {
    if (!games) {
      updateTable();
      return;
    }
    populateGamesTable(games);
    loadQueue(function(queue) {
      if (!queue) {
        updateTable();
        return;
      }
      updateSnapjoin(queue, games);
      setTimeout(updateTable, 400);
    })
  });
}

function filterTable() {
  input = document.getElementById("search-game");
  filter = input.value.toUpperCase();
  table = document.getElementById("poker-tables");
  tr = table.getElementsByTagName("tr");
  for (i = 1; i < tr.length; i++) {
    td = tr[i].getElementsByTagName("td")[1];
    txtValue = td.textContent;
    if (txtValue.toUpperCase().indexOf(filter) > -1) {
      tr[i].style.display = "";
    } else {
      tr[i].style.display = "none";
    }
  }
}

function listActiveGames() {
  list = document.getElementById("active-games");
  list.innerHTML = "";

}

function activateTokenMode() {
  let tokenMode = document.getElementsByClassName("token-mode");
  let chipMode = document.getElementsByClassName("chip-mode");
  for(let i = 0; i < chipMode.length; i++){
    chipMode[i].classList.addClass("hidden");
  }
  for(let i = 0; i < tokenMode.length; i++){
    tokenMode[i].classList.removeClass("hidden");
  }
}

function activateChipMode() {
  let tokenMode = document.getElementsByClassName("token-mode");
  let chipMode = document.getElementsByClassName("chip-mode");
  for(let i = 0; i < chipMode.length; i++){
    console.log(chipMode[i]);
    chipMode[i].classList.removeClass("hidden");
  }
  for(let i = 0; i < tokenMode.length; i++){
    tokenMode[i].classList.addClass("hidden");
  }
}

function openLoginForm() {
  loginForm = document.getElementById("login-form");
  loginForm.classList.remove("hidden");
}

function closeLoginForm() {
  loginForm = document.getElementById("login-form");
  loginForm.classList.add("hidden");
}

function signup() {

}

function submitLoginForm(callback) {
  email = document.getElementById("email");
  loginForm = document.getElementById("login-form");
  loginForm.classList.add("hidden");

  if(!callback) {
    callback = function() {}
  }
  xhr = new XMLHttpRequest();
  xhr.open('GET', 'api/login/' + email.value);
  xhr.onload = function() {
    if (xhr.status === 200) {
      callback();
    } else {
      console.error(xhr.status, xhr.responseText);
      callback();
    }
  }
  xhr.send();
}

//populateGamesTable([{type:"NLHE", city:"Anchorage", active_players:3, table_size:6, bigblind:200},
//  {type:"PLO", city:"Seattle", active_players:2, table_size:6, bigblind:100}]);
//i = setInterval(updateTable, 300);
updateTable();
populateSnapjoin();
