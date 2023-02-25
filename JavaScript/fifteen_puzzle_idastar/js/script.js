/**
 * STRUCTURE OF THIS FILE:
 * 1. IDA* related data structures and functions
 * 2. Drag handlers
 * 3. Initialization of the puzzle
 * 4. Getters and setters that support the game 
 * 5. A priority queue implementation using binary heap
 */

/**
 * IDA* related data structures
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
 * IDA* related functions 
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

// count how many node has been visited
const count = new Map(); // Map<number, number>: <depth, numOfNodesVisited>
let currCount = 0;

/**
 * The traditional recursive DFS function that searches for a solution.
 * @param {number} limit how many deeper can we go from here (root)
 * @param {Node} root root node of currently seraching tree
 * @returns null if no solution found, or the solution node
 */
const dfs = (limit, root) => {
    currCount++;
    if (isCorrect(root.state)) {
        return root;
    }
    if (limit === 0) return null;

    const potentialMoves = getPotentialMoves(root.state);
    const nextStates = potentialMoves.map(move => {
        if (root.state.lastTo !== move[0]) { // PRUN!!
            const newState = {
                emptyCellIndex: move[0],
                content: swap(root.state.content, move[0], move[1]),
                lastTo: move[1],
            };
            return new Node(newState, [...root.path, move]);
        } else {
            return null;
        }
    }).filter(s => s !== null);

    // Empirically, this increases performance, but it is not considered standard IDA*.
    nextStates.sort((a, b) => a.cost - b.cost)


    // 短路遍历（找到一个解）
    for (let nextState of nextStates) {
        if (getHeuristics(nextState.state) <= limit - 1) {  // PRUN!!
            result = dfs(limit - 1, nextState);
            if (result !== null) {
                return result;
            }
        }
    }

    return null;
}




/**
 * IDA* function that calls the dfs function with iterative depth. (+1 
 * for each iteration)
 * @returns The solution.
 */
const idaStar = async () => {
    count.clear();
    const start = new Date().getTime();
    let limit = getHeuristics(state);
    let solution;
    const root = new Node({ ...state }, []);
    root.lastTo = -1; // this is to help prevent traversing back during dfs
    do {
        document.getElementById("depthLimit").innerText = `搜索深度：${limit}`;
        await sleep(50);
        solution = dfs(limit, root);
        limit += 2; // this is due to some weird duality
        count.set(limit - 2, currCount);
        currCount = 0;
    } while (solution === null);

    const end = new Date().getTime();
    let result = `搜索时间：${(end - start) - 50 * count.size}ms`;
    count.forEach((value, key) => {
        result += `\n深度 ${key} 搜索节点数：${value}`;
    });
    result += `\n需要 ${limit - 2} 步还原\n\n点击以播放动画`;

    document.getElementById("depthLimit").innerText = '';

    alert(result);

    return solution;
}

const start = async () => {
    disableDragAndDrop();
    document.getElementById("solveBtn").innerText = "求解中...";
    await sleep(1);
    const solution = await idaStar();
    document.getElementById("solveBtn").innerText = "播放动画中...";
    await playAnimation(solution);
    enableDragAndDrop();
    document.getElementById("solveBtn").innerText = "IDA*求解";
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
// const aPuzzle = ["", "2", "7", "4", "9", "3", "5", "8", "1", "10", "12", "11", "13", "14", "6", "15"]; // 28 steps
// const aPuzzle = ['1', '2', '8', '4', '5', '6', '3', '', '15', '11', '12', '7', '9', '13', '10', '14']; // 28 steps
// const aPuzzle = ['5', '1', '3', '2', '14', '6', '10', '4', '', '13', '11', '7', '15', '12', '9', '8']; // easy 30 steps
const aPuzzle = ['1', '', '15', '10', '14', '11', '7', '5', '3', '4', '13', '2', '8', '12', '9', '6']; // default 57 steps


async function customSetUp() {
    reset(numbers);
    disableDragAndDrop();
    document.getElementById("customSetUpBtn").innerText = "打乱中...";
    await sleep(100);
    await messUp(document.getElementById("numInput").value);
    enableDragAndDrop();
    document.getElementById("customSetUpBtn").innerText = "自定义步数打乱";
    alert("打乱完成！\n点击【IDA*求解】开始求解");
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
