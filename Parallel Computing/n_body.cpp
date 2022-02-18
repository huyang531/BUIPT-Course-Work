/**
 * @file n_body.cpp
 * @author 胡杨 (michael531@bupt.edu.cn)
 * @brief This program simulates n-body problem and saves results in ./data directory.
 * @version 2.1
 * @date 2021-12-17
 *
 * @copyright Copyright (c) 2021
 *
 * @remark This program is written in c++17 standard. To compile, place this file,
 * vector2d.h, and vector2d.cpp in a same directory, then run:
 * g++ -std=c++1z *.cpp -o nbody
 */
#include <filesystem>
#include <random>
#include <stack>
#include <string>
#include <vector>

#include "vector2d.h"

#define NUM_PLANETS 1000   // number of planets
#define TIME_PER_ITER 0.1  // (TIME_PER_ITER * v) gives the distance moved in one iteration
#define NUM_ITERS 4000     // number of iterations
#define UNI_SIZE 8000      // 2 * UNI_SIZE is the length of the edge of the largest square
// #define INIT_VELOCITY      // whether planets have initial velocity
// #define INIT_SUN           // whether to init the sun
// #define CIRCLE             // whether to init planets in a circle
// #define VERBOSE  // whether to show a lot of output during computation

struct Planet {
    Vector2d pos;   // position
    Vector2d v;     // velocity
    Vector2d a;     // acceleration
    double mass{};  // mass
} planets[NUM_PLANETS + 1],
    updatedPlanets[NUM_PLANETS + 1];  // to store temp changes and update planets[] altogether

struct Node {
    int son[4] = {-1, -1, -1, -1};  // children nodes
    int pid = -1;                   // planet id
    int parent = -1;                // parent node id
    Vector2d center;                // center is either planets[pid].pos or the center of gravity
    double mass{};                  // mass is either planets[pid].mass or the accumulated mass of children nodes
};

const double minDist = 1;  // two planets cannot come closer than minDist, this parameter does not have much effect though
const double eps = 1e-9;   // epsilon (infinitesimal) - if you want to have some fun, set minDist to eps 😄
#ifdef INIT_SUN
const double G = 6.67408e-4;  // gravitational constant (in real life, it's 6.67408e-11)
#else
const double G = 6.67408;
#endif

std::random_device rd;      // random device
std::mt19937 engine(rd());  // random number generator
// std::default_random_engine engine; // this default generator gives predictable sequence
std::uniform_real_distribution<double> unidist_uni(-UNI_SIZE, UNI_SIZE);                        // random pos in universe
std::uniform_real_distribution<double> unidist_v(-100. / TIME_PER_ITER, 100. / TIME_PER_ITER);  // random velocity
std::normal_distribution<double> norm(1000., 50.);                                              // random mass
std::uniform_real_distribution<double> unidist_ang(0, 2 * 3.1415926);                           // random angle from 0 ~ 2π

std::vector<Node> tree;
bool out[NUM_PLANETS + 1];

/**
 * Find acceleration between two planets, on planet 1.
 */
Vector2d getAcceleration(double m1, double m2, const Vector2d& pos1, const Vector2d& pos2) {
    double dis = pos1.dis_between(pos2);
    if (dis < minDist) {  // if two planets become too close, they won't attract each other anymore
        return Vector2d(0, 0);
    }
    double f = G * (m1 * m2) / pow(dis, 2),
           fx = f * (pos2.x - pos1.x) / dis,
           fy = f * (pos2.y - pos1.y) / dis;

    return Vector2d(fx / m1, fy / m1);
}

/**
 * Update acceleration of planet represented by node tree[c]
 * @param c
 */
void updateAcceleration(int c) {
    const Node& cn = tree[c];  // current node
    if (cn.pid == -1) {
        std::cerr << "I DON'T HAVE A VELOCITY!" << std::endl;
        exit(-9);
    }
    // Note that we can't just modify planets[cn.pid] because that would interfere
    // with other planets on the same layer.
    Planet me = Planet(planets[cn.pid]);  // me as a new planet
    me.a = Vector2d();                    // reset acceleration

    // accumulate acceleration from brothers
    for (const auto& n : tree[cn.parent].son) {
        if (tree[n].pid != -1 && tree[n].pid != cn.pid) {  // if it's a planet and it's not me
            Planet& other = planets[tree[n].pid];
            me.a = me.a + getAcceleration(me.mass, other.mass, me.pos, other.pos);
        }
    }

    // accumulate acceleration from ancestor nodes
    int curr = cn.parent;
    while (curr != 0) {  // as long as it's not the root node
        // accumulate acceleration from curr's brothers
        for (const auto& b : tree[tree[curr].parent].son) {  // my parent must have 4 valid sons, so no -1 check here
            if (b != curr && tree[b].mass > 0) {             // if that node is not myself and it's valid
                me.a = me.a + getAcceleration(me.mass, tree[b].mass, me.pos, tree[b].center);
            }
        }
        // backtrace to parent
        curr = tree[curr].parent;
    }

    // in Git's language, stage change
    updatedPlanets[cn.pid] = me;
}

/**
 * Insert a node in the tree.
 *
 * @remark starting position: position of the bottom left corner of a square
 * @remark center position: the center position of a square, determined by sx, sy, and size
 * @remark real planet: a planet in planets[]
 * @remark abstract planet: a planet that represents a set of planets in close proximity.
 * It's represented as a node in the tree, and cannot be found in planets[].
 *
 * @param i tree[i] is current node
 * @param j planet[j] is the node to add to current node's sub-tree
 * @param sx starting position's x
 * @param sy starting position's y
 * @param size size of this square
 */
void insertNode(int i, int j, double sx, double sy, double size) {
    double mass = 0;
    Vector2d center = Vector2d();
    size /= 2;
    if (size < eps) {
        std::cerr << "[ERROR] Cannot split square anymore!" << std::endl;
        exit(-10);
    }

    // get center pos and index of sub-square to which planets[j] belongs
    int cx = planets[j].pos.x < sx + size,
        cy = planets[j].pos.y < sy + size,
        c_index = (cx << 1) | cy;

    if (tree[i].son[0] == -1) {   // if current node has no children
        if (tree[i].pid != -1) {  // if current node represents a real planet
            // make current node an abstract planet
            for (int k = 0; k < 4; k++) {
                tree.emplace_back();               // add node to tree
                tree[i].son[k] = tree.size() - 1;  // mark node as child
                tree[tree[i].son[k]].parent = i;   // init child's parent
            }

            // find index of children to which the real planet belongs
            int rx = planets[tree[i].pid].pos.x < sx + size,
                ry = planets[tree[i].pid].pos.y < sy + size,
                r_index = (rx << 1) | ry;

            // move the real planet to that child
            insertNode(
                tree[i].son[r_index],
                tree[i].pid,
                sx + !rx * size,
                sy + !ry * size,
                size);

            // add planets[j] to its place
            insertNode(
                tree[i].son[c_index],
                j,
                sx + !cx * size,
                sy + !cy * size,
                size);
            tree[i].pid = -1;  // mark current node as an abstract planet
        } else {               // current node is an abstract node
            // make it a node that represent the real planet planets[j]
            tree[i].pid = j;
            mass = planets[j].mass;
            center = planets[j].pos;
        }
    } else {  // current node has 4 children
        insertNode(
            tree[i].son[c_index],
            j,
            sx + !cx * size,
            sy + !cy * size,
            size);
    }

    /**
     * Update center and mass.
     * @remark Since insertNode() is called recursively, parent nodes are updated after
     * children nodes are.
     */
    if (tree[i].son[0] != -1) {      // if current node has children, then
        for (int k : tree[i].son) {  // re-calculate its center and mass
            Node& n = tree[k];
            if (n.mass > 0) {  // some nodes may have not yet been initialized
                // find weighed center
                center = (center * mass + n.center * n.mass) / (mass + n.mass);
                mass += n.mass;
            }
        }
    }

    tree[i].center = center;
    tree[i].mass = mass;
}

void initUniverse() {
    // create folder for data storage (this function already has error handling)
    std::filesystem::create_directory("./data");

#ifdef INIT_SUN
    // init the sun
    planets[0].mass = 999999999999.;
    planets[0].pos = Vector2d(0, 0);
#endif

    // init other planets
    for (int i = 0; i < NUM_PLANETS + 1; i++) {
        if (planets[i].mass > 0) continue;  // skip if sun is already initialized

            // init position
#ifdef CIRCLE  // init planets in a circle
        double ang = unidist_ang(engine),
               radius = unidist_uni(engine);
        planets[i].pos.x = radius * cos(ang);
        planets[i].pos.y = radius * sin(ang);
#else  // or, in a square
        planets[i].pos.x = unidist_uni(engine);
        planets[i].pos.y = unidist_uni(engine);
#endif
        // init mass
        planets[i].mass = abs(norm(engine)) + 100.;

#ifdef INIT_VELOCITY
        planets[i].v.x = unidist_v(engine);
        planets[i].v.y = unidist_v(engine);
#endif
    }

    // write init data (and meta data)
    std::fstream fout;
    fout.open("./data/init.csv", std::ios::out);
    if (!fout.is_open()) {
        std::cerr << "File cannot open!" << std::endl;
        exit(-3);
    }
    fout << UNI_SIZE << ',' << TIME_PER_ITER << ',' << NUM_ITERS << ',' << G << '\n';
    for (const auto& p : planets) {
        fout << p.mass << ',' << p.pos << ',' << p.v << '\n';
    }
    fout.close();

#ifdef VERBOSE
    // print init velocity
    std::cout << "[Init]" << std::endl;
    for (int i = 0; i < NUM_PLANETS + 1; i++) {
        std::cout << "\tPlanet " << i << "pos: " << planets[i].pos << "  v: " << planets[i].v << std::endl;
    }
#endif

    // init tree
    tree.emplace_back();                         // add root node
    for (int i = 0; i < NUM_PLANETS + 1; i++) {  // add other nodes
        insertNode(0, i, -UNI_SIZE, -UNI_SIZE, 2 * UNI_SIZE);
    }
}

int main() {
    // std::cout << "minDist: ";
    // std::cin >> minDist;
    initUniverse();

    for (int it = 0; it < NUM_ITERS; it++) {  // for every iteration
        // update acceleration
        // declare new variables every iteration so no reset is needed
        std::vector<bool> visited(tree.size(), false);  // just in case
        std::vector<int> changed;                       // mark staged changes
        std::stack<int> stack;
        stack.push(0);
        while (!stack.empty()) {
            // dfs all leaves
            int curr = stack.top();
            stack.pop();
            if (!visited[curr]) {
                visited[curr] = true;
                for (const auto n : tree[curr].son) {
                    if (tree[n].mass > 0) {       // if it's a valid son
                        if (tree[n].pid == -1) {  // if the son has more sons
                            stack.push(n);
                        } else {  // if the son is a leaf
                            visited[n] = true;
                            updateAcceleration(n);
                            changed.emplace_back(tree[n].pid);
                        }
                    }
                }
            }
        }

        // in Git's language, commit staged changes
        for (const auto& i : changed) {
            planets[i] = std::move(updatedPlanets[i]);
        }

        // update position and velocity
        for (int i = 0; i < NUM_PLANETS + 1; i++) {
            if (!out[i]) {  // if planet is still in scope
                planets[i].pos = planets[i].pos + planets[i].v * TIME_PER_ITER + planets[i].a * 0.5 * pow(TIME_PER_ITER, 2);
                planets[i].v = planets[i].v + planets[i].a * TIME_PER_ITER;
                // check if planet flies outside the universe
                if (abs(planets[i].pos.x) > UNI_SIZE ||
                    abs(planets[i].pos.y) > UNI_SIZE) {
                    out[i] = true;
                    planets[i].v = Vector2d();
                    std::cout << "Planet " << i << " flew out the universe at iter " << it << "!" << std::endl;
                }
            }
        }

#ifdef VERBOSE
        std::cout << "[Iter " << it << "]" << std::endl;
        for (int i = 0; i < NUM_PLANETS + 1; i++) {
            if (!out[i]) {
                std::cout << "\tPlanet " << i << ": " << planets[i].pos << std::endl;
            }
        }
#endif

        // write data
        std::fstream fout;
        fout.open("./data/" + std::to_string(it) + ".csv", std::ios::out);
        if (!fout.is_open()) {
            std::cerr << "File won't open" << std::endl;
            exit(-3);
        }
        for (int i = 0; i < NUM_PLANETS + 1; i++) {
            if (out[i]) {
                // consider doing something here
                fout << planets[i].pos << ',' << planets[i].v - planets[i].a * 0.5 * TIME_PER_ITER << '\n';

            } else {
                fout << planets[i].pos << ',' << planets[i].v - planets[i].a * 0.5 * TIME_PER_ITER << '\n';
            }
        }
        fout.close();

        // create new tree
        tree.clear();
        tree.emplace_back();
        for (int i = 0; i < NUM_PLANETS + 1; i++) {
            if (!out[i]) {
                insertNode(0, i, -UNI_SIZE, -UNI_SIZE, 2 * UNI_SIZE);
            }
        }
    }

    int cnt = std::count_if(std::cbegin(out), std::cend(out), [](bool c) { return c; });
    std::cout << "Out: " << cnt << std::endl;
}