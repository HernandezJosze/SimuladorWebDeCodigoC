document.addEventListener('DOMContentLoaded', function ( ){
    let result;
    const inputProgram = document.getElementById("inputProgram"),
          outputProgram = document.getElementById("outputProgram"),
          container = document.getElementById("containerSVG");

    const posX = 30, posY = 30, tam = 60, addCY = 120;
    let cx = posX, cy = posY, instructions = [ ], currentInst = 0, data = new Map( ),
        data_backup = new Map( ), stack = [], stack_backup = [], text = "", memoryText = [text],
        MAX_X, MAX_Y, elementoMasDerecha = 0, elementoMasAbajo = 0;
    function cleanAll( ){
        btnStop.click( );
        cx = posX, cy = posY, instructions = [ ], currentInst = 0, data = new Map( );
        data_backup = new Map( ), stack = [], stack_backup = [], text = "", memoryText = [text];
        outputProgram.value = "";
        MAX_X = container.clientWidth, MAX_Y =  container.clientHeight;
        elementoMasAbajo = 0, elementoMasDerecha = 0;
        while(container.hasChildNodes( )){
            container.removeChild(container.lastChild);
        }
    }
    Module.onRuntimeInitialized = function( ) {
        let interpret = Module.cwrap("interpreta", "string", ["string", "string"]);
        document.getElementById("btnRunInterpret").onclick = function( ) {
            result = interpret(document.getElementById("inputCode").value, inputProgram.value);
            initialization( );
        }
    };
    window.addEventListener('resize', ( ) =>{
        const simulador = document.getElementById("simulador");
        simulador.setAttribute("style", "min-width:" + Math.max(Math.min(1900, elementoMasDerecha + 300), 1050) + "px; "
                                        + "min-height:" + Math.max(Math.min(1050, elementoMasAbajo + 300), 600) + "px;");
        MAX_X = container.clientWidth, MAX_Y =  container.clientHeight;
    });
    function delay(seconds){
        return new Promise(function(resolve){
            setTimeout(resolve,seconds * 1000);
        });
    }
    async function resaltaSalida( ){
        if(!outputProgram.classList.contains('salidaActivaW') && outputProgram.classList.contains('whiteMode')){
            outputProgram.classList.toggle('salidaActivaW');
            await delay(0.15);
            outputProgram.classList.toggle('salidaActivaW');
        }else if(!outputProgram.classList.contains('salidaActivaD') && outputProgram.classList.contains('darkModeContainer')){
            outputProgram.classList.toggle('salidaActivaD');
            await delay(0.15);
            outputProgram.classList.toggle('salidaActivaD');
        }
    }

    const btnDark = document.getElementById('btnDarkMode');
    btnDark.addEventListener('click', ( ) => {
        document.getElementById('body').classList.toggle('darkModeBody');
        container.classList.toggle('darkModeContainer');
        container.classList.toggle('whiteMode');
        inputProgram.classList.toggle('darkModeEditor');
        document.getElementById('inputCode').classList.toggle('darkModeEditor');
        outputProgram.classList.toggle('darkModeContainer');
        outputProgram.classList.toggle('whiteMode');
        btnDark.innerText = btnDark.textContent === "Modo oscuro" ? "Modo claro" : "Modo oscuro";
    });

    function createSvg(type, svg_attributes, properties) {
        let node = document.createElementNS("http://www.w3.org/2000/svg", type);
        for (const name in svg_attributes) {
            node.setAttributeNS(null, name, svg_attributes[name]);
        }
        for (const name in properties) {
            node[name] = properties[name];
        }
        return node;
    }

    function initialization( ) {
        cleanAll();

        const simulador = document.getElementById("simulador");
        simulador.setAttribute("style", "min-width:" + 1050 + "px; "
            + "min-height:" + 600 + "px;");

        const ids = ["btnNextInst", "btnPlay"];
        for (let id of ids) {
            document.getElementById(id).removeAttribute("disabled");
        }
        btnPrev.setAttribute('disabled', '');

        const line = result.split('\04\n'); //separador de instruccion
        for (let instruction of line) {
            instructions.push(instruction.split('\01'));// Separador de token
        }
        if (instructions[getLastPos(instructions)][0] === "") {
            instructions.pop(); // eliminamos el salto de linea del final
        }
    }

    function auxPlay( ) {
        if (currentInst < instructions.length && !btnStop.hasAttribute('disabled')) {
            const timer = setTimeout(() => {
                executeNextInstruction();
                auxPlay();
            }, 800 / parseInt(document.getElementById('speed').value));
            btnStop.addEventListener('click', () => {
                clearTimeout(timer);
            });
        } else {
            btnStop.click();
            btnNext.click();
        }
    }

    const btnPlay = document.getElementById('btnPlay');
    btnPlay.addEventListener('click', ( ) => {
        btnStop.removeAttribute("disabled");
        const ids = ["btnNextInst", "btnPrevInst", "btnPlay"];
        for (const id of ids) {
            document.getElementById(id).setAttribute("disabled", '');
        }
        auxPlay();
    });

    const btnStop = document.getElementById('btnStop');
    btnStop.addEventListener('click', function( ) {
        this.setAttribute('disabled', '');
        const ids = ["btnNextInst", "btnPrevInst", "btnPlay"];
        for (const id of ids) {
            document.getElementById(id).removeAttribute("disabled");
        }
    });

    const btnNext = document.getElementById("btnNextInst");
    btnNext.addEventListener('click', ( ) => {
        if (currentInst < instructions.length) {
            btnPrev.removeAttribute('disabled');
            executeNextInstruction();
        } else if (currentInst === instructions.length) {
            btnNext.setAttribute('disabled', '');
            btnPlay.setAttribute('disabled', '');
        }
    });

    function isArray(name) {
        let index = name.indexOf("[");
        if (index === -1) {
            return {'name': name, 'var': index};
        } else {
            const NAME = name.substr(0, index);
            let VAR = "";
            ++index;
            while (name[index] !== "]") {
                VAR += name[index++];
            }
            return {'name': NAME, 'var': VAR};
        }
    }

    function executeNextInstruction( ) {
        const command = instructions[currentInst][0];   //command type/name name/value value
        if (command === "CREA") {
            const type = instructions[currentInst][1];
            const name = instructions[currentInst][2];
            stack.push({
                'name': name,
                'version': 'v' + (data.has(name) ? data.get(name).length : 0)
            });
            if (!data.has(name)) {
                data.set(name, []);
            }
            if (type === "ARR") {
                const tam = instructions[currentInst][3];
                const value = instructions[currentInst].length === 5 ? instructions[currentInst][4].split(',') : [];
                data.get(name).push({
                    'tam': tam,
                    'vars': []
                });
                createArr(name, value);
            } else if (type === "VAR") {
                const value = instructions[currentInst].length === 4 ? instructions[currentInst][3] : '?';
                data.get(name).push({
                    'tam': -1,
                    'values': [value]
                });
                createVar(name);
            }
        } else if (command === 'ESCRIBE') {
            const res = isArray(instructions[currentInst][1]);//name
            const value = instructions[currentInst][2];
            const last = getLastPos(data.get(res.name));
            let id;
            if (res.var === -1) {// es var
                data.get(res.name)[last].values.push(value);
                id = `${res.name}v${last}`;
            } else { // es array
                data.get(res.name)[last].vars[res.var].values.push(value);
                id = `${res.name}v${last}[${res.var}]`;
            }
            modifyBox(id, value);
        } else if (command === 'DESTRUYE') {
            let veces = instructions[currentInst][1];
            while (veces--) {
                const variable = stack.pop();
                stack_backup.push(variable);

                deleteVarArrOfHTML(variable.name, variable.version);
                if (!data_backup.has(variable.name)) {
                    data_backup.set(variable.name, []);
                }
                data_backup.get(variable.name).push(data.get(variable.name).pop());
            }
        } else if (command === 'IMPRIME') {
            const value = instructions[currentInst][1];
            text += value;
            memoryText.push(text);
            outputProgram.value = text;
            outputProgram.scrollTop = outputProgram.scrollHeight;
            resaltaSalida();
        } else {
            btnPrev.setAttribute('disabled', '');
            btnNext.setAttribute('disabled', '');
            btnPlay.setAttribute('disabled', '');
            btnStop.setAttribute('disabled', '');
            outputProgram.value = command;
            resaltaSalida();
            return;
        }
        currentInst++;
    }

    const btnPrev = document.getElementById('btnPrevInst');
    btnPrev.addEventListener('click', ( ) => {
        if (currentInst === 1) {
            btnPrev.setAttribute('disabled', '');
        }
        btnNext.removeAttribute('disabled');
        btnPlay.removeAttribute('disabled');
        backPrevInstruction();
    });

    function backPrevInstruction( ) {
        currentInst--;
        const command = instructions[currentInst][0];
        if (command === "CREA") {
            const variable = stack.pop();
            deleteVarArrOfHTML(variable.name, variable.version);
            data.get(variable.name).pop();
        } else if (command === 'ESCRIBE') {
            const res = isArray(instructions[currentInst][1]); //name
            const last = data.get(res.name).length - 1;
            let id, value;
            if (res.var === -1) { // entonces es una variable
                data.get(res.name)[last].values.pop();
                value = data.get(res.name)[last].values[getLastPos(data.get(res.name)[last].values)];
                id = `${res.name}v${last}`;
            } else { //es un arreglo
                data.get(res.name)[last].vars[res.var].values.pop();
                value = data.get(res.name)[last].vars[res.var].values[getLastPos(data.get(res.name)[last].vars[res.var].values)];
                id = `${res.name}v${last}[${res.var}]`;
            }
            modifyBox(id, value);
        } else if (command === 'DESTRUYE') {
            let veces = instructions[currentInst][1];
            while (veces--) {
                const variable = stack_backup.pop();
                stack.push(variable);
                data.get(variable.name).push(data_backup.get(variable.name).pop());
                recovery(variable.name);
            }
        } else if (command === 'IMPRIME') {
            memoryText.pop();
            outputProgram.value = text = memoryText[memoryText.length - 1];
            resaltaSalida();
        }
    }

    function recovery(name) {
        if (data.get(name)[getLastPos(data.get(name))].tam === -1) {
            createVar(name);
        } else {
            createArr(name, [], false);
        }
    }

    function getLastPos(arr) {
        return arr.length - 1;
    }

    function createArr(name, values, primera_vez = true) {
        const num = data.get(name)[getLastPos(data.get(name))].tam;
        if (cx + tam + tam * num >= MAX_X) {
            if (cy + addCY >= MAX_Y || posX + tam * num + tam >= MAX_X) {
                btnStop.click();
                btnNext.setAttribute('disabled', '');
                data.get(name).pop();
                stack.pop();
                --currentInst;
                alert("El arreglo es demasiado grande");
                return;
            }
            cx = posX;
            cy += addCY;
        }
        const realName = stack[getLastPos(stack)].name + stack[getLastPos(stack)].version;
        //Create the name of the array
        let node = createSvg("text", {
            "x": cx,
            "y": cy + 5 + tam / 2,
            "text-anchor": "middle",
            "class": "small"
        }, {"textContent": name + ':'});
        node.id = `svg|${realName}|`;
        container.appendChild(node);
        cx += tam / 2;

        //Create array
        if (primera_vez) {
            const insert = (!values.length ? '?' : '0');
            while (values.length < num) {
                values.push(insert);
            }
            for (let index = 0; index < num; ++index) {
                createSpace(index, realName, values[index]);
                cx += tam;
                data.get(name)[getLastPos(data.get(name))].vars.push({
                    'tam': -1,
                    'values': [values[index]]
                });
            }
            cx += tam;
        } else {
            createArrAux(name);
        }
    }

    function createArrAux(name) {
        const num = data.get(name)[getLastPos(data.get(name))].tam;
        const realName = stack[getLastPos(stack)].name + stack[getLastPos(stack)].version;
        //Create array
        for (let index = 0; index < num; ++index) {
            const last = data.get(name)[getLastPos(data.get(name))].vars.length - 1;
            const lastV = data.get(name)[getLastPos(data.get(name))].vars[last].values.length - 1;
            createSpace(index, realName, data.get(name)[getLastPos(data.get(name))].vars[last].values[lastV]);
            cx += tam;
        }
        cx += tam;
    }

    function deleteVarArrOfHTML(name, version) {
        const last = getLastPos(data.get(name));
        if (data.get(name)[last].tam === -1) {
            deleteVar(name + version);
        } else {
            deleteArr(name + version, data.get(name)[last].tam);
        }
    }

    function deleteArr(name, num) {
        document.getElementById(`svg|${name}|`).remove();
        while (num > 0) {
            num -= 1;
            const ids = [`svg|${name}[${num}]|box`, `svg|${name}[${num}]|value`, `svg|${name}[${num}]|name`];
            for (let id of ids) {
                document.getElementById(id).remove();
            }
            cx -= tam;
        }
        cx = (cx - tam - tam / 2);
        repositionCY();
    }

    function createVar(name) {
        if (cx + tam >= MAX_X) {
            if (cy + addCY >= MAX_Y) {
                btnStop.click();
                btnNext.setAttribute('disabled', '');
                --currentInst;
                data.get(name).pop();
                stack.pop();
                alert("Espacio lleno");
                return;
            }
            cx = posX;
            cy += addCY;
        }
        const last = data.get(name).length - 1;
        const lastV = data.get(name)[last].values.length - 1;
        createSpace(name, '', data.get(name)[last].values[lastV]);
        cx += 2 * tam;
    }

    function deleteVar(name) {
        const ids = [`svg|${name}|box`, `svg|${name}|value`, `svg|${name}|name`];
        for (const id of ids) {
            document.getElementById(id).remove();
        }
        cx = cx - 2 * tam;
        repositionCY();
    }

    function repositionCY( ) {
        const child = container.lastChild;
        if (cx === posX && child) {
            cx = parseInt(child.getAttribute('x')) + tam + tam / 2;
            cy = cy !== 30 ? cy - addCY : cy;
        }
    }

    function createSpace(name, father, value) {
        const realName = stack[getLastPos(stack)].name + stack[getLastPos(stack)].version;
        // Create variable's name
        let node, NAME = (father === '' ? `${realName}` : `${father}[${name}]`);
        let id = `svg|${NAME}|`;
        node = createSvg("text", {
            "x": cx + tam / 2,
            "y": cy + 15 + tam,
            "text-anchor": "middle",
            "class": "small"
        }, {"textContent": name});
        node.id = id + 'name';
        container.appendChild(node);

        //Create box variable
        node = createSvg('rect', {
            "rx": 2,
            "x": cx,
            "y": cy,
            "width": tam,
            "height": tam,
            "fill": "white",
            "stroke": "black"
        });
        elementoMasDerecha = Math.max(elementoMasDerecha, cx + tam);
        elementoMasAbajo = Math.max(elementoMasAbajo, cy + 5);
        node.id = id + 'box';
        container.appendChild(node);

        //Create content
        node = createSvg("text", {"x": cx + tam / 2, "y": cy + 5 + tam / 2, "text-anchor": "middle", "class": "small"});
        node.id = id + 'value';
        container.appendChild(node);
        modifyBox(NAME, value);
    }

    let lastActive = undefined;
    function modifyBox(name, value) {
        let box = document.getElementById(`svg|${name}|box`), node = document.getElementById(`svg|${name}|value`);
        if (node) {
            node.textContent = value;
            if (lastActive) {
                lastActive.removeAttribute('class');
            }
            box.setAttribute('class', 'active');
            lastActive = box;
        }
    }
});