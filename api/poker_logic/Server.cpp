#include "Server.h"
#include "JSON.h"
#include "APIFunctions.h"

#include <iostream>
#include <thread>

string get_player_id(string session_id) {
  session_id_to_player_id_mutex.lock_shared();
  string player_id = "";
  if (session_id_to_player_id.count(session_id)) {
    player_id = session_id_to_player_id[session_id];
  }
  session_id_to_player_id_mutex.unlock_shared();
  return player_id;
}

void set_player_id(string session_id, string player_id) {
  session_id_to_player_id_mutex.lock();
  session_id_to_player_id[session_id] = player_id;
  session_id_to_player_id_mutex.unlock();
}

mutex rand_mutex;

int generate_int() {
  int ret;
  rand_mutex.lock();
  ret = rand();
  rand_mutex.unlock();
  return ret;
}

string generate_id() {
  return to_string(generate_int());
}

void init_server() {
  srand(time(NULL));
  game_settings qs = {nlhe, ring, 6, -1, 2};
  queue[qs];
}

string get_games() {
  string ret = "[\n";
  all_games_mutex.lock_shared();
  bool first = true;
  for(auto g : all_games) {
    if (first) {
      first = false;
    } else {
      ret += ",\n";
    }
    ret += g.second->to_json(1);
  }
  if (!first) {
    ret += "\n";
  }
  all_games_mutex.unlock_shared();
  ret += "]\n";
  return ret;
}

// requires: tables[table_id].hand_sim.isHandOver()
void start_new_hand(string table_id) {
  // Awarding money from prevous hand
  // Get buttonLocation and vector<int> stacks from previous handsim
  // Clear out any players that are in game.leaving_players
  // Add waiting_players into the game
  // Create new handsim based on previous handsim
  // break up game and add players to queue
  all_tables_mutex.lock();
  all_games_mutex.lock();
  Table* table = all_tables[table_id];
  HandSimulation* hand_sim = &table->hand_sim;
  hand_sim->awardWinners();
  int buttonLocation = hand_sim->getButtonLocation();
  vector<int> stacks = hand_sim->getStacks();
  vector<string> player_ids = table->player_ids;
  string& game_id = table->game_id;
  Game* game = all_games[game_id];
  //remove leaving_players
  for(string& player_id : game->leaving_players) {
    for(int i = 0; i < player_ids.size(); i++) {
      if (player_ids[i] == player_id) {
        player_ids.erase(player_ids.begin() + i);
        stacks.erase(stacks.begin() + i);
        i--;
      }
    }
  }
  //add waiting_players
  // TODO: Fix for tournaments
  for(string& player_id : game->waiting_players) {
    player_ids.push_back(player_id);
    stacks.push_back(100*game->settings.big_blind);
  }
  //updating table and game
  game->player_ids = player_ids;
  game->leaving_players.clear();
  game->waiting_players.clear();
  table->player_ids = player_ids;
  // TODO: dead button, dead smallBlind
  buttonLocation += 1;
  buttonLocation %= game->settings.table_size;
  if(player_ids.size() > game->settings.table_size/2){
    int big_blind = game->settings.big_blind; // game->settings.format == ring ? game->settings.big_blind : game->big_blind
    HandSimulation hs(big_blind, buttonLocation, stacks);
    hs.initBettingRound();
    table->hand_sim = move(hs);
  } else {
    all_games.erase(game_id);
    all_tables.erase(table_id);
    delete table;
    delete game;
    delete game_mutexes[game_id];
    delete table_mutexes[table_id];
    game_mutexes.erase(game_id);
    table_mutexes.erase(table_id);
  }
  all_games_mutex.unlock();
  all_tables_mutex.unlock();
}

string player_act(string session_id, string table_id, string action, int bet_size) {
  //poker action posts
  string player_id = get_player_id(session_id);
  if (player_id == "") {
    return error_json("Player not logged in");
  }
  string ret = "{}\n";
  all_tables_mutex.lock_shared();
  if (all_tables.count(table_id)) {
    table_mutexes[table_id]->lock();
    vector<string>& player_ids = all_tables[table_id]->player_ids;
    HandSimulation& hs = all_tables[table_id]->hand_sim;
    if (hs.isHandOver()) {
      ret = error_json("Hand is already over");
    } else if (player_ids[hs.getCurrentTurn()] != player_id) {
      ret = error_json("It is not your turn");
    } else {
      if (action == "raise") {
        if (bet_size <= hs.getCurrentBet()) {
          ret = error_json("Raise must be larger than the curent bet");
        } else if (!hs.isValidBet(bet_size)) {
          ret = error_json("Raise size is not valid");
        } else {
          hs.Bet(bet_size);
        }
      } else if (action == "bet") {
        if (hs.getCurrentBet() != 0) {
          ret = error_json("Bet cannot be made as a bet already exists");
        } else if (!hs.isValidBet(bet_size)) {
          ret = error_json("Bet size is not valid");
        } else {
          hs.Bet(bet_size);
        }
      } else if (action == "call") {
        if (hs.canCheck()) {
          ret = error_json("Cannot call if check is available");
        } else {
          hs.Call();
        }
      } else if (action == "check") {
        if (!hs.canCheck()) {
          ret = error_json("Cannot check, the current bet is " + to_string(hs.getCurrentBet()));
        } else {
          hs.Check();
        }
      } else if (action == "fold") {
        hs.Fold();
      } else {
        ret = error_json(action + " is not a valid action");
      }
      while(hs.isBettingRoundOver()) {
        hs.endBettingRound();
        if (hs.isHandOver()) {
          break;
        }
        hs.initBettingRound();
      }
      if (hs.isHandOver()) {
        thread t([table_id]{
          this_thread::sleep_for(chrono::milliseconds(2000));
          start_new_hand(table_id);
        });
      }
    }
    table_mutexes[table_id]->unlock();
  }
  all_tables_mutex.unlock_shared();
  return ret;
}

// queue_mutex must be locked in order to call this
void create_tables_from_queues() {
  //fill existing nonempty tables
  all_games_mutex.lock_shared();
  for(auto& elem : all_games){
    const string& game_id = elem.first;
    game_mutexes[game_id]->lock();
    Game* g = elem.second;
    game_settings settings = g->settings;
    while( queue[settings].size() > 0
        && g->player_ids.size() + g->waiting_players.size() < g->settings.table_size
    ) {
      string player_id = queue[settings].back();
      queue[settings].pop_back();
      g->waiting_players.push_back(player_id);
    }
    game_mutexes[game_id]->unlock();
  }
  all_games_mutex.unlock_shared();
  //fill existing nonempty tables before populating new tables
  for(auto& specific_queue : queue) {
    game_settings settings = specific_queue.first;
    if (settings.format != ring) {
      throw "We haven't coded tournaments yet";
    }
    vector<string>& player_ids = specific_queue.second;
    while(player_ids.size() > settings.table_size/2){
      //make new game
      vector<string> new_table_player_ids;
      while (new_table_player_ids.size() < settings.table_size && player_ids.size() > 0) {
        new_table_player_ids.push_back(player_ids.back());
        player_ids.pop_back();
      }
      string table_id = generate_id();
      string game_id = generate_id();
      HandSimulation hs(settings.big_blind, 0, vector<int>(settings.table_size, 100*settings.big_blind));
      hs.initBettingRound();
      Table* table = new Table{table_id, game_id, new_table_player_ids, hs};
      Game* game = new Game(game_id, "NYC", settings, {table_id}, new_table_player_ids);

      all_tables_mutex.lock();
      all_tables[table_id] = table;
      table_mutexes[table_id] = new shared_mutex;
      all_tables_mutex.unlock();

      all_games_mutex.lock();
      all_games[game_id] = game;
      game_mutexes[game_id] = new shared_mutex;
      all_games_mutex.unlock();

      player_ids.clear();
    }
  }
}

string add_to_queue(string session_id, string type, string format, int table_size, int buy_in_or_big_blind) {
  //player joining game
  string player_id = get_player_id(session_id);
  if (player_id == "") {
    return error_json("Player not logged in");
  }
  string invalid_settings = error_json("A queue with those settings does not exist");
  if (!is_game_type(type) || !is_game_format(format)) {
    return invalid_settings;
  }

  string ret = "{}";
  game_settings settings = {to_game_type(type), to_game_format(format), table_size, -1, -1};
  if (settings.format == ring) {
    settings.big_blind = buy_in_or_big_blind;
  } else {
    settings.buy_in = buy_in_or_big_blind;
  }
  queue_mutex.lock();
  if (queue.count(settings)) {
    vector<string>& player_ids = queue[settings];
    player_ids.push_back(player_id);
    create_tables_from_queues();
  } else {
    ret = invalid_settings;
  }
  queue_mutex.unlock();
  return ret;
}

string get_game_from_id(string game_id) {
  string response;
  all_games_mutex.lock_shared();
  if(all_games.count(game_id) > 0){
    game_mutexes[game_id]->lock_shared();
    Game& game = *all_games[game_id];
    response = game.to_json() + "\n";
    game_mutexes[game_id]->unlock_shared();
  } else {
    response = error_json("Game ID Not Found");
  }
  all_games_mutex.unlock_shared();

  return response;
};

string get_table_from_id(string table_id) {
  string response;
  all_tables_mutex.lock_shared();
  if(all_tables.count(table_id)){
    table_mutexes[table_id]->lock_shared();
    Table& thetable = *all_tables[table_id];
    response = thetable.to_json() + "\n";
    table_mutexes[table_id]->unlock_shared();
  } else {
    response = error_json("Table ID Not Found");
  }
  all_tables_mutex.unlock_shared();

  return response;
};
