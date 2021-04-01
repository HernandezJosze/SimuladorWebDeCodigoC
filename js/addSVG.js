let idNodes = [0];
const container = document.getElementById("containerSVG");
const button = document.getElementById("agregaSVG");
let width = 300, height = 300;
function create_svg(tipo, atributos_svg, propiedades) {
   let nodo = document.createElementNS("http://www.w3.org/2000/svg", tipo);
   for (let nombre in atributos_svg) {
      nodo.setAttributeNS(null, nombre, atributos_svg[nombre]);
   }
   for (let nombre in propiedades) {
      nodo[nombre] = propiedades[nombre];
   }
   return nodo
}

button.addEventListener("click", function() {
   idNodes.push(idNodes[idNodes.length - 1] + 1);
   let svg = document.createElementNS("http://www.w3.org/2000/svg", "svg");
   svg.id = "svg" + idNodes[idNodes.length - 1];
   svg.style.width = width + 'px';
   svg.style.height = height + 'px';
   container.appendChild(svg);
   svg.appendChild(create_svg("circle", { "cx": width / 2, "cy": height / 2, "r": 80 }));
   svg.appendChild(create_svg("text", { "x": width / 2, "y": height / 2 + 25, "text-anchor":"middle"}, { "textContent": "SVG" }));
});
