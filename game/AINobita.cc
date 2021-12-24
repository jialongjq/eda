#include "Player.hh"
#include <stack>
#include <queue>
#include <unordered_map>
#include <unordered_set>

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Nobita


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  /**
   * Types and attributes for your player can be defined here.
   */

  vector<Dir> directions = {Up, Left, Down, Right};

  struct citizen_info {
    Pos barricade_pos;
    bool moved;
  };

  unordered_map<int, citizen_info> citizen_info;

  set<Pos> tracked_pos;
  set<Pos> pending_pos;

  int distance(const Pos& p1, const Pos& p2) {
    int i = 0, j = 0;
    if (int(p1.i) < int(p2.i)) i = p2.i - p1.i;
    else i = p1.i - p2.i;
    if (int(p1.j) < int(p2.j)) j = p2.j - p1.j;
    else j = p1.j - p2.j;
    return i + j;
  }

  // Returns true if the weapon is better than the current weapon of the warrior
  bool better_weapon(int id, WeaponType weapon) {
    WeaponType c_weapon = citizen(id).weapon;
    if (c_weapon == Bazooka) return weapon == Bazooka;
    else if (c_weapon == Gun) return weapon == Bazooka;
    else if (c_weapon == Hammer) return weapon == Bazooka or weapon == Gun;
    return true;
  }

  bool can_make_step(Pos p) {
    return (pos_ok(p) and cell(p).type != Building and cell(p).id == -1 and (cell(p).b_owner == -1 or cell(p).b_owner == me()));
  }

  pair<Pos, Pos> bfs_resource(int id, map<Pos, Pos>& visited) {
    Pos c_pos = citizen(id).pos;

    queue<Pos> pending;

    pending.push(c_pos);
    visited[c_pos] = c_pos;

    bool nearest = true;
    Pos p_second = Pos(-1, -1);

    while (not pending.empty()) {
      Pos p_aux = pending.front(); pending.pop();
      
      BonusType p_bonus = cell(p_aux).bonus;
      WeaponType p_weapon = cell(p_aux).weapon;
      if (p_bonus != NoBonus or p_weapon != NoWeapon) {
        if (citizen(id).type == Builder) {
          if (p_weapon == NoWeapon) {
            tracked_pos.insert(p_aux);
            return make_pair(p_aux, p_second);
          }
          else if (p_weapon == Bazooka) {
            return make_pair(p_aux, p_second);
          }
          else if (p_weapon == Gun) {
            if (nearest) {
              p_second = p_aux;
              nearest = false;
            }
          }
        }
        else if (citizen(id).type == Warrior) {
          if (citizen(id).life < warrior_ini_life() / 2) {
            if (p_bonus == Food or better_weapon(id, p_weapon)) {
              if (p_bonus == Food) tracked_pos.insert(p_aux);
              return make_pair(p_aux, p_second);
            }
          }
          else {
            if (better_weapon(id, p_weapon)) {
              return make_pair(p_aux, p_second);
            }
            else if (p_bonus == Money or p_bonus == Food) {
              if (nearest) {
                p_second = p_aux;
                nearest = false;
              }
            }
          }
        }
      }

      for (int i = 0; i < 4; ++i) {
        Pos p_next = p_aux + directions[i];
        if (visited.find(p_next) == visited.end() and can_make_step(p_next)) {
          if (tracked_pos.find(p_next) == tracked_pos.end() and pending_pos.find(p_next) == pending_pos.end()) {
            visited[p_next] = p_aux;
            pending.push(p_next);
          }
        }
      }

    }
    return make_pair(Pos(-1, -1), p_second);
  }

  Dir dir_from_p1_to_p2(const Pos& p1, const Pos&p2) {
    if (p1.i == p2.i) {
      if (p1.j < p2.j) return Right;
      return Left;
    }
    else {
      if (p1.i < p2.i) return Down;
      return Up;
    }
  }

  Dir opposite_dir(Dir d) {
    if (d == Up) return Down;
    if (d == Down) return Up;
    if (d == Left) return Right;
    return Left;
  }

  Dir optional_dir(Pos p, Dir d) {
    if (d == Up or d == Down) { // Enemigo vertical, solo puedo ir en horizontal
      if (can_make_step(p + Left)) return Left;
      if (can_make_step(p + Right)) return Right;
    }
    if (d == Left or d == Right) { // Enemigo horizontal, solo puedo ir en vertical
      if (can_make_step(p + Up)) return Up;
      if (can_make_step(p + Down)) return Down;
    }
    return d; // No me queda otra que enfrentarme
  }

  // Pre-condition: There exists a path from p1 to p2 in path
  Pos first_step_in_path(Pos p1, Pos p2, map<Pos, Pos>& path) {
    Pos previous_step = path[p2];
    if (previous_step == p1) return p2;
    return first_step_in_path(p1, previous_step, path);
  }

  bool can_build(Pos p) {
    Cell c;
    if (pos_ok(p)) c = cell(p);
    else return false;
    return (c.type == Street and c.bonus == NoBonus and c.weapon == NoWeapon
            and (c.b_owner == -1 or c.b_owner == me()) and c.id == -1); 
  }

  void build_barricade(int id) {
    Pos c_pos = citizen(id).pos;
    if (citizen_info[id].barricade_pos == Pos(-1, -1)) {
      if (can_build(c_pos + Up)) {
        citizen_info[id].barricade_pos = (c_pos + Up);
        build(id, Up);
      }
      else if (can_build(c_pos + Down)) {
        citizen_info[id].barricade_pos = (c_pos + Down);
        build(id, Down);
      }
      else if (can_build(c_pos + Left)) {
        citizen_info[id].barricade_pos = (c_pos + Left);
        build(id, Left);
      }
      else if (can_build(c_pos + Right)) {
        citizen_info[id].barricade_pos = (c_pos + Right);
        build(id, Right);
      }
    }
    else {
      build(id, dir_from_p1_to_p2(citizen(id).pos, citizen_info[id].barricade_pos));
    }
  }

  pair<bool, Dir> resource_near(int id) {
    Pos p = citizen(id).pos;
    if (can_make_step(p + Up) and (cell(p + Up).bonus != NoBonus or cell(p + Up).weapon != NoWeapon)) return make_pair(true, Up);
    if (can_make_step(p + Down) and (cell(p + Down).bonus != NoBonus or cell(p + Down).weapon != NoWeapon)) return make_pair(true, Down);
    if (can_make_step(p + Left) and (cell(p + Left).bonus != NoBonus or cell(p + Left).weapon != NoWeapon)) return make_pair(true, Left);
    if (can_make_step(p + Right) and (cell(p + Right).bonus != NoBonus or cell(p + Right).weapon != NoWeapon)) return make_pair(true, Right);
    return make_pair(false, Up);
  }

  bool can_find_enemy(Pos p, int id) {
    return (pos_ok(p) and cell(p).type == Street and (cell(p).id == -1 or citizen(cell(p).id).player != me()) and cell(p).resistance == -1);
  }

  bool can_find_citizen(Pos p) {
    return (pos_ok(p) and cell(p).type == Street and (cell(p).id == -1 or cell(p).id != -1));
  }

  bool can_beat(int my_id, int enemy_id) {
    Citizen enemy = citizen(enemy_id);
    
    if (enemy.player != me() and enemy.weapon == citizen(my_id).weapon) {
      return citizen(my_id).life >= enemy.life;
    }
    return enemy.player != me() and (enemy.type == Builder or not better_weapon(my_id, enemy.weapon));
  }

  pair<bool, Dir> enemy_near(int id) {

    Pos p = citizen(id).pos;
    Dir d_weakest = Up;
    bool first = true;

    for (Dir d : directions) {
      Pos p_next = p + d;
      if (pos_ok(p_next) and cell(p_next).id != -1 and citizen(cell(p_next).id).player != me() and cell(p_next).resistance == -1) {
        if (first) {
          first = false;
          d_weakest = d;
        }
        else if (can_beat(cell(p + d_weakest).id, cell(p + d).id)) {
          d_weakest = d;
        }
      }
    }
    if (first) return make_pair(false, d_weakest);
    return make_pair(true, d_weakest);
  }

  pair<bool, Pos> enemy_near2(int id) {

    Pos p = citizen(id).pos;

    queue<Pos> near;

    near.push(p + Up + Up);
    near.push(p + Down + Down);
    near.push(p + Left + Left);
    near.push(p + Right + Right);
    near.push(p + Up + Left);
    near.push(p + Up + Right);
    near.push(p + Down + Left);
    near.push(p + Down + Right);
    
    Pos p_strongest = Pos(-1, -1);

    bool found = false;

    while (not near.empty()) {
      Pos p_aux = near.front(); near.pop();
      if (pos_ok(p_aux) and cell(p_aux).id != -1 and citizen(cell(p_aux).id).player != me() and cell(p_aux).resistance == -1) {
        if (not found) {
          found = true;
          p_strongest = p_aux;
        }
        else if (can_beat(cell(p_aux).id, cell(p_strongest).id)) {
          p_strongest = p_aux;
        }
      }
    }
    if (found) return make_pair(true, p_strongest);
    return make_pair(false, p_strongest);
  }


  pair<Pos, Pos> bfs_enemy(int id, map<Pos, Pos>& visited, bool not_kill) {
    Pos c_pos = citizen(id).pos;
    visited[c_pos] = c_pos;
    queue<Pos> pending;
    pending.push(c_pos);

    bool nearest = true;
    Pos p_second = Pos(-1, -1);

    while (not pending.empty()) {
      Pos p_aux = pending.front(); pending.pop();

      int enemy_id = cell(p_aux).id;
      if (not_kill) {
        if (enemy_id != -1 and citizen(enemy_id).player != me()) {
          return make_pair(p_aux, p_second);
        }
      }
      else if ((enemy_id != -1 and can_beat(id, enemy_id)) or cell(p_aux).weapon == Bazooka) {
        return make_pair(p_aux, p_second);
      }
      else if (nearest and (cell(p_aux).bonus != NoBonus or cell(p_aux).weapon != NoWeapon)) {
        nearest = false;
        p_second = p_aux;
      }

      for (int i = 0; i < 4; ++i) {
        Pos p_next = p_aux + directions[i];
        if (visited.find(p_next) == visited.end() and can_find_enemy(p_next, id)) {
          visited[p_next] = p_aux;
          pending.push(p_next);
        }
      }
    }
    return make_pair(Pos(-1, -1), p_second);
  }

  void search_enemy(int id) {
    map<Pos, Pos> visited;

    pair<Pos, Pos> nearest_pos = bfs_enemy(id, visited, false);

    Pos r_pos = Pos(-1, -1);

    if (nearest_pos.first != Pos(-1, -1)) r_pos = nearest_pos.first;
    else if (nearest_pos.second != Pos(-1, -1)) r_pos = nearest_pos.second;

    if (r_pos != Pos(-1, -1)) {
      Pos c_pos = citizen(id).pos;
      Pos next_pos = first_step_in_path(c_pos, r_pos, visited);
      move(id, dir_from_p1_to_p2(c_pos, next_pos));
    }
  }

  void search_resource(const int& id) {
    map<Pos, Pos> visited;

    pair<Pos, Pos> nearest_pos = bfs_resource(id, visited);

    Pos r_pos = Pos(-1, -1);

    if (nearest_pos.first != Pos(-1, -1)) r_pos = nearest_pos.first;
    else if (nearest_pos.second != Pos(-1, -1)) r_pos = nearest_pos.second;

    if (r_pos != Pos(-1, -1)) {
      Pos c_pos = citizen(id).pos;
      Pos next_pos = first_step_in_path(c_pos, r_pos, visited);
      if (can_make_step(next_pos)) {
        pending_pos.insert(next_pos);
        move(id, dir_from_p1_to_p2(c_pos, next_pos));
      }
    }
    else search_enemy(id);
  }

  Dir direction_to_escape(Pos p_me, Pos p_enemy) {
    if (p_enemy == p_me + Up + Up) {
      if (can_make_step(p_me + Down)) return Down;
      else return optional_dir(p_me, Down);
    }
    else if (p_enemy == p_me + Down + Down) {
      if (can_make_step(p_me + Up)) return Up;
      else return optional_dir(p_me, Up);
    }
    else if (p_enemy == p_me + Left + Left) {
      if (can_make_step(p_me + Right)) return Right;
      else return optional_dir(p_me, Right);
    }
    else if (p_enemy == p_me + Right + Right) {
      if (can_make_step(p_me + Left)) return Left;
      else return optional_dir(p_me, Left);
    }
    else if (p_enemy == p_me + Up + Left) {
      if (can_make_step(p_me + Right)) return Right;
      else if (can_make_step(p_me + Down)) return Down;
      if (can_make_step(p_me + Up)) return Up;
      return Left;
    }
    else if (p_enemy == p_me + Up + Right) {
      if (can_make_step(p_me + Left)) return Left;
      else if (can_make_step(p_me + Down)) return Down;
      if (can_make_step(p_me + Up)) return Up;
      return Right;
    }
    else if (p_enemy == p_me + Down + Left) {
      if (can_make_step(p_me + Right)) return Right;
      else if (can_make_step(p_me + Up)) return Up;
      if (can_make_step(p_me + Down)) return Down;
      return Left;
    }
    else { // == Down + Right
      if (can_make_step(p_me + Left)) return Left;
      else if (can_make_step(p_me + Up)) return Up;
      if (can_make_step(p_me + Down)) return Down;
      return Right;
    }
  }

  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {

    int n_movements = (num_rounds_per_day()/2) - round() % (num_rounds_per_day()/ 2); // Numbers of movements avaliable during the day/night

    vector<int> builders_id = builders(me());

    vector<int> warriors_id = warriors(me());

    if (is_day()) {

      tracked_pos.clear();
      pending_pos.clear();

      for (int id : builders_id) {
        if (round() % num_rounds_per_day() == 0) {
        citizen_info[id].barricade_pos = Pos(-1, -1);
        }
      }

      for (int id : warriors_id) {
        if (citizen(id).life > 0) {
          if (citizen(id).weapon == Bazooka and citizen(id).life == warrior_ini_life()) {
            search_enemy(id);
          }
          else search_resource(id);
        }
      }
      for (int id : builders_id) {
        if (citizen(id).life > 0) {
          pair<bool, Dir> resource = resource_near(id);
          if (resource.first) {
            if (n_movements == 1) move(id, resource.second);
            else {
              if (cell(citizen(id).pos + resource.second).weapon == Bazooka) {
                map<Pos, Pos> visited;
                pair<Pos, Pos> p_enemy = bfs_enemy(id, visited, true);
                if (p_enemy.first != Pos(-1, -1) and citizen(cell(p_enemy.first).id).type == Warrior and distance(citizen(id).pos, p_enemy.first) <= 3) {
                  move(id, resource.second);
                }
                else build_barricade(id);
              }
              else move(id, resource.second);
            }
          }
          else search_resource(id);
        }
      }
    }
    else if (is_night()) {

      unordered_set<int> moved;

      vector<int> warriors_builders_id = warriors_id;

      warriors_builders_id.insert(warriors_builders_id.end(), builders_id.begin(), builders_id.end());
      
      
      // PRIMERO PRIORIZO LOS GUERREROS Y BUILDERS QUE PUEDEN MATAR
      // SI NO PUEDEN MATAR Y EL ENEMIGO ESTÁ A UNA DISTANCIA DE 1, A ESCAPAR

      for (int id : warriors_builders_id) {
        if (citizen(id).life > 0) {
          pair<bool, Dir> enemy_found = enemy_near(id);
          if (enemy_found.first) {
            Pos c_pos = citizen(id).pos;
            Citizen enemy = citizen(cell(c_pos + enemy_found.second).id);
            if (can_beat(id, enemy.id)) {
              move(id, enemy_found.second);
              moved.insert(id);
            }
            else {
              Dir opposite = opposite_dir(enemy_found.second);
              if (can_make_step(c_pos + opposite)) move(id, opposite);
              else move(id, optional_dir(c_pos, opposite));
              moved.insert(id);
            }
          }
        }
      }

      // LUEGO PRIORIZO LOS GUERREROS Y BUILDERS QUE TIENEN QUE ESCAPAR SI HAY ENEMIGOS
      // FUERTES A UNA DISTANCIA DE 2

      for (int id : warriors_builders_id) {
        if (citizen(id).life > 0 and moved.find(id) == moved.end()) {
          pair<bool, Pos> enemy_found = enemy_near2(id);
          if (enemy_found.first) {
            Citizen enemy = citizen(cell(enemy_found.second).id);
            if (not can_beat(id, enemy.id)) {
              move(id, direction_to_escape(citizen(id).pos, enemy_found.second));
              moved.insert(id);
            }
          }
        }
      }

      // EL RESTO A BUSCAR ENEMIGOS (y si no encuentran, a por recursos, que ya está implementado en el bfs de enemigos)

      for (int id : warriors_builders_id) {
        if (citizen(id).life > 0 and moved.find(id) == moved.end()) {
          search_enemy(id);
        }
      }
    }
  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
