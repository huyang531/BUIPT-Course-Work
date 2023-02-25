/**
 * STRUCTURE OF THIS FILE:
 * 1. A* related data structures and functions
 * 2. Drag handlers
 * 3. Initialization of the puzzle
 * 4. Getters and setters that support the game 
 * 5. A priority queue implementation using binary heap
 */

/**
 * A* related data structures
 */
class Node {
    /**
     * @param {state} state 
     * @param {list<tuple: [number, number]>} path 
     */
    constructor(state, path) {
        this.state = state;
        this.cost = path.length + getHeuristics(this.state);
        this.path = path;
    }
}

/**
 * A* related functions 
 */

/**
 * Returns potential move tuples. Indices range from [0, 15].
 * @param {state object} state 
 * @returns tuple: [fromIndex, toIndex]
 */
const getPotentialMoves = (state) => {
    const { emptyCellIndex } = state;
    const emptyCellPos = getCellPos(emptyCellIndex);
    const potentialMoves = [];

    // get the potential moves of the empty cell
    if (emptyCellPos[0] > 0) {
        potentialMoves.push([getCellIndex(emptyCellPos[0] - 1, emptyCellPos[1]), emptyCellIndex]);
    }
    if (emptyCellPos[0] < 3) {
        potentialMoves.push([getCellIndex(emptyCellPos[0] + 1, emptyCellPos[1]), emptyCellIndex]);
    }
    if (emptyCellPos[1] > 0) {
        potentialMoves.push([getCellIndex(emptyCellPos[0], emptyCellPos[1] - 1), emptyCellIndex]);
    }
    if (emptyCellPos[1] < 3) {
        potentialMoves.push([getCellIndex(emptyCellPos[0], emptyCellPos[1] + 1), emptyCellIndex]);
    }

    return potentialMoves;
}

const getManhattanDistance = (pos1, pos2) => {
    return Math.abs(pos1[0] - pos2[0]) + Math.abs(pos1[1] - pos2[1]);
}

/**
 * For each value, calculate the Manhattan distance between its current
 * position and its final position and then sum them up.
 * @param {state object} state 
 * @returns the heuristic cost
 */
const getHeuristics = (state) => {
    let heuristicCost = 0;
    numbers.forEach((number, index) => {
        if (number !== "") {
            const currentIndex = state.content.indexOf(number);
            heuristicCost += getManhattanDistance(getCellPos(index), getCellPos(currentIndex));
        }
    })
    return heuristicCost;
}

const swap = (content, a, b) => {
    let copy = [...content];
    const temp = copy[a];
    copy[a] = copy[b];
    copy[b] = temp;
    return copy;
}

const aStar = () => {
    const start = new Date().getTime();

    const pq = new PriorityQueue();
    const visited = new Map(); // key: state.content; value: true, if visited
    const root = new Node({ ...state }, []);
    pq.enqueue(root, root.cost);
    let node = pq.dequeue();

    while (!isCorrect(node.state)) {
        // mark visited
        visited.set(JSON.stringify(node.state.content), true);

        // get potential moves
        const potentialMoves = getPotentialMoves(node.state);

        // make new nodes and enqueue them
        potentialMoves.forEach(move => {
            const newState = {
                emptyCellIndex: move[0],
                content: swap(node.state.content, move[0], move[1])
            };
            const newNode = new Node(newState, [...node.path, move]);
            if (!(visited.get(JSON.stringify(node.state.content) === true))) {
                pq.enqueue(newNode);
            }
        });

        node = pq.dequeue();
    }
    const end = new Date().getTime();

    alert(`已搜索节点数： ${visited.size}\nOPEN 集中剩余节点数： ${pq.size()}\n\
耗时：${end - start}ms\n还原所需步数：${node.path.length}\n\n点击以播放动画`);

    return node;
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

const playAnimation = async (solution) => {
    for (let i = 0; i < solution.path.length; i++) {
        const move = solution.path[i];
        const from = document.getElementById(`li${move[0]}`);
        const to = document.getElementById(`li${move[1]}`);

        // swap
        const temp = from.innerText;
        from.innerText = to.innerText;
        to.innerText = temp;

        // update state
        updateState();

        // sleep
        await sleep(1000);
    }
}

const start = async () => {
    disableDragAndDrop();
    document.getElementById("solveBtn").innerText = "求解中...";
    await sleep(100);
    const solution = aStar();
    document.getElementById("solveBtn").innerText = "播放动画中...";
    await playAnimation(solution);
    enableDragAndDrop();
    document.getElementById("solveBtn").innerText = "A*求解";
    alert("还原完成");
}

const messUp = async (num) => {
    let lastMove, randomIndex;
    for (let i = 0; i < num; i++) {
        const potentialMoves = getPotentialMoves(state);
        do {
            randomIndex = Math.floor(Math.random() * potentialMoves.length);
        } while (lastMove && potentialMoves[randomIndex][0] === lastMove[1]);
        const move = potentialMoves[randomIndex];
        const from = document.getElementById(`li${move[0]}`);
        const to = document.getElementById(`li${move[1]}`);

        // swap
        const temp = from.innerText;
        from.innerText = to.innerText;
        to.innerText = temp;

        // update state
        updateState();

        // sleep
        await sleep(500);
        lastMove = move;
    }
}

/**
 * Drag-related handlers 
 */

const dragstartHandler = (ev) => {
    ev.dataTransfer.setData("text/plain", ev.target.id);
    ev.dataTransfer.dropEffect = "move";
}

const dragoverHandler = (ev) => {
    ev.preventDefault();
}

const dropHandler = (ev) => {
    ev.preventDefault();
    // get the id of the target and add the moved element to the target's DOM
    const data = ev.dataTransfer.getData("text/plain");
    ev.target.innerText = document.getElementById(data).innerText;

    // empty old cell
    document.getElementById(data).innerText = "";
}

const dragendHandler = (ev) => {
    // remove all of the drag data
    ev.dataTransfer.clearData();

    // update state and reset drag/drop attribute values
    updateState();
    setDroppable(ul);
    setDraggable(ul);
}

/**
 * Initialize grid tiles
 */

// select the list items
let ul = document.querySelectorAll("li");
const numbers = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", ""];
// const aPuzzle = ['1', '11', '2', '3', '6', '7', '4', '', '5', '9', '10', '8', '13', '14', '15', '12']; // 16 steps
// const aPuzzle = ['5', '3', '6', '4', '9', '1', '8', '11', '', '15', '2', '12', '13', '14', '10', '7']; // 26 steps
// const aPuzzle = ['5', '2', '1', '3', '10', '6', '8', '7', '13', '9', '11', '4', '14', '', '15', '12']; // 26 steps
// const aPuzzle = ['1', '2', '8', '4', '5', '6', '3', '', '15', '11', '12', '7', '9', '13', '10', '14']; // hard 28 steps
const aPuzzle = ['', '2', '7', '4', '9', '3', '5', '8', '1', '10', '12', '11', '13', '14', '6', '15']; // default 28 steps
// const aPuzzle = ['5', '1', '3', '2', '14', '6', '10', '4', '', '13', '11', '7', '15', '12', '9', '8']; // easy 30 steps
// const aPuzzle = ['1', '', '15', '10', '14', '11', '7', '5', '3', '4', '13', '2', '8', '12', '9', '6']; // 57 steps

async function customSetUp() {
    reset(numbers);
    disableDragAndDrop();
    document.getElementById("customSetUpBtn").innerText = "打乱中...";
    await sleep(100);
    await messUp(document.getElementById("numInput").value);
    enableDragAndDrop();
    document.getElementById("customSetUpBtn").innerText = "自定义步数打乱";
    alert("打乱完成！\n点击【A*求解】开始求解");
}

function reset(pattern) {
    fillGrid(ul, pattern);
    setId(ul); // this might be called multiple times but it's OK
    updateState();

    // set up the droppable and draggable contents
    setDroppable(ul);
    setDraggable(ul);
}

function setUp() {
    // shuffle numbers
    let shuffledNumbers = shuffle(numbers);
    fillGrid(ul, shuffledNumbers);
    setId(ul); // this might be called multiple times but it's OK
    updateState();

    // set up the droppable and draggable contents
    setDroppable(ul);
    setDraggable(ul);
}

const state = {}
state.content = numbers;

/**
 * Getters 
 */
const updateState = () => {
    // refresh state
    state.content = getState(ul);
    for (let i = 0; i < state.content.length; i++) {
        if (state.content[i] == "") {
            state.emptyCellIndex = i;
            break;
        }
    }

    // reset style
    ul.forEach((li) => {
        if (li.innerText === "") {
            li.setAttribute("class", "empty");
        } else {
            li.classList.remove("empty");
        }
    });
}

const getState = (items) => {
    const content = [];
    items.forEach(item => {
        content.push(item.innerText);
    });
    return content;
}

/**
 * Get the 2D position of a cell. 
 * The function takes a cellIndex ranges from [0, 15], and then
 * transform it into a 2D position.
 * @returns The 2D position of the cell. E.g. (0, 0) is the top-left cell.
 */
const getCellPos = (cellIndex) => {
    const cellNumber = cellIndex + 1;
    const cellRow = Math.ceil(cellNumber / 4);
    const cellCol = 4 - (4 * cellRow - cellNumber);
    return [cellRow - 1, cellCol - 1];
}

/**
 * Opposite of getCellPos().
 * @param {number} row 
 * @param {number} col 
 * @returns an index ranges from [0, 15]
 */
const getCellIndex = (row, col) => {
    return 4 * row + col;
}

/**
 * Check if game is finished.
 * @returns If game is finished.
 */
const isCorrect = (someState) => {
    return JSON.stringify(numbers) == JSON.stringify(someState.content);
}

/**
 * Setters
 */

/**
 * Shuffle an array in a way that guarantees a solution
 * @returns a shuffled arr
 */
const shuffle = (arr) => {
    const copy = [...arr];
    do {
        // loop over the array
        for (let i = 0; i < copy.length; i++) {
            // for each index i, pick a random index j
            let j = parseInt(Math.random() * copy.length);
            // swap elements at i and j
            let temp = copy[i];
            copy[i] = copy[j];
            copy[j] = temp;
        }
    } while (!isSolvable(copy))
    return copy;
}

const isSolvable = (arr) => {
    let numOfInversions = 0;
    // get number of inversions
    for (let i = 0; i < arr.length; i++) {
        for (let j = i + 1; j < arr.length; j++) {
            if (arr[i] > arr[j]) numOfInversions++;
        }
    }
    return numOfInversions % 2 == 0;
}

// set text for list items
const fillGrid = (items, numbers) => {


    items.forEach((item, i) => {
        item.innerText = numbers[i];
    })
}

// set ids for list items
const setId = (items) => {
    for (let i = 0; i < items.length; i++) {
        items[i].setAttribute("id", `li${i}`);
    }
}

const setDroppable = (items) => {
    items.forEach((item) => {
        // remove old droppable
        item.setAttribute("ondrop", "");
        item.setAttribute("ondragover", "");

        if (!item.innerText) {
            item.setAttribute("ondrop", "dropHandler(event)");
            item.setAttribute("ondragover", "dragoverHandler(event)");
        }
    })
}

const setDraggable = (items) => {
    // set only the cells adjacent to the empty cell draggable
    const potentialMoves = getPotentialMoves(state);
    const potentialValues = potentialMoves.map(move => {
        return state.content[move[0]];
    });

    items.forEach((item) => {
        // remove old draggable
        item.setAttribute("draggable", "");
        item.setAttribute("ondragstart", "");
        item.setAttribute("ondragend", "");

        if (potentialValues.includes(item.innerText)) {
            item.setAttribute("draggable", "true");
            item.setAttribute("ondragstart", "dragstartHandler(event)");
            item.setAttribute("ondragend", "dragendHandler(event)");
        }
    })
}

const enableDragAndDrop = () => {
    setDraggable(ul);
    setDroppable(ul);
    document.querySelectorAll("button").forEach((button) => {
        button.removeAttribute("disabled");
    });
    document.getElementById("numInput").removeAttribute("disabled");
}

const disableDragAndDrop = () => {
    ul.forEach((item) => {
        item.setAttribute("draggable", "");
        item.setAttribute("ondragstart", "");
        item.setAttribute("ondragend", "");
        item.setAttribute("ondrop", "");
        item.setAttribute("ondragover", "");
    });
    document.querySelectorAll("button").forEach((button) => {
        button.setAttribute("disabled", "");
    });
    document.getElementById("numInput").setAttribute("disabled", "");
}

/**
 * A priority queue implementation using Binary Heap
 * A Binary Heap is a complete binary tree, where the children nodes
 * are always larger than the parent node.
 */
class PriorityQueue {
    constructor() {
        this.values = []; // list of Nodes
    }

    size() {
        return this.values.length;
    }

    /**
     * Enqueue a node. Find a right parent bottom-up.
     * @param {Node} val 
     */
    enqueue(val) {
        this.values.push(val);
        let index = this.values.length - 1;
        const current = this.values[index];

        while (index > 0) {
            const parentIndex = Math.floor((index - 1) / 2);
            const parent = this.values[parentIndex];
            // move until a correct position is found
            if (current.cost > parent.cost) break;

            // swap parent with current
            this.values[index] = parent;
            this.values[parentIndex] = current;
            index = parentIndex;
        }
    }

    /**
     * Dequeue a node. Place the last node at the root and then re-order top-down.
     * @returns The node with the highest priority (smallest cost)
     */
    dequeue() {
        const min = this.values[0];
        const end = this.values.pop();

        if (this.values.length > 0) { // if queue is not empty
            // place last node at the root
            this.values[0] = end;
            let index = 0;

            // re-order top-down
            const length = this.values.length;
            const element = this.values[0];
            while (true) {
                let leftChildIndex = 2 * index + 1;
                let rightChildIndex = 2 * index + 2;
                let leftChild, rightChild;
                let swap = null;

                // examine if and where to swap
                if (leftChildIndex < length) { // if left child exists
                    leftChild = this.values[leftChildIndex];
                    if (leftChild.cost < element.cost) {
                        swap = leftChildIndex;
                    }
                }
                if (rightChildIndex < length) { // if right child exists
                    rightChild = this.values[rightChildIndex];
                    if (
                        (swap === null && rightChild.cost < element.cost) ||
                        (swap !== null && rightChild.cost < leftChild.cost)
                    ) {
                        swap = rightChildIndex;
                    }
                }

                if (swap === null) break;

                // swap
                this.values[index] = this.values[swap];
                this.values[swap] = element;
                index = swap;
            }
        }
        return min;
    }
}