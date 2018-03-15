class Brain
  constructor: (graph, data) ->
    window.addEventListener 'resize', -> location.reload()

    @graph = graph
    @graph.backgroundColor('transparent').enableNodeDrag(false)
    @setupNodes()
    @setupLinks()
    @setupHover()
    @setupClick()
    @graph.graphData(data)

    setTimeout(->
      $('.logo').show()
      $('.loader').removeClass('loader--animate').hide()
    , 100)

  setupNodes: ->
    @graph
      .nodeThreeObject(@buildNode)
      .nodeLabel (node) =>
        return "<span class='agent-hover-label'>#{node.name}<span>";

  buildNode: (node) ->
    if node.isAgent
      sphere = Object3d.agent(node.name_short)
    else
      sphere = Object3d.intent(node.name_short)
    return sphere;

  setupLinks: ->
    @graph
      .linkColor (link) ->
        if link.isBetweenAgents
          return 0x4bdbff
        else
          return 0xffffff
      .linkWidth (link) ->
        if link.isBetweenAgents
          return 0.65
        if link.isBetweenAgentAndIntent
          return 0.25
      .linkOpacity .35
      .linkDirectionalParticleResolution 1
      .linkDirectionalParticles (link) ->
        if link.isBetweenAgents
          return 1
        else if link.isBetweenIntents
          return 1
        else
          return null
      .linkDirectionalParticleWidth (link) ->
        if link.isBetweenAgents
          return 1.05
        else if link.isBetweenIntents
          return .35
        else
          return null

  setupClick: ->
    @graph
      .onNodeClick (node) =>
        @goTo(node)
        return true

  setupHover: ->
    @graph
      .onNodeHover (node, prevNode) ->
        if node # In
          if node.isAgent
            node.__threeObj.material.color = { r:  52 / 255, g: 202 / 255, b: 239 / 255 }
          else
            node.__threeObj.material.color = { r: 200 / 255, g: 200 / 255, b: 200 / 255 }
        else # Out
          if prevNode.isAgent
            prevNode.__threeObj.material.color = { r:  75 / 255, g: 219 / 255, b: 255 / 255 }
          else
            prevNode.__threeObj.material.color = { r: 238 / 255, g: 238 / 255, b: 238 / 255 }

  goTo: (node) ->
    node_position   = { x: node.x, y: node.y, z: node.z }
    target_position = { x: node.x + 50, y: node.y + 50, z: node.z + 50 }

    t1 = new TWEEN.Tween(@graph.getTbControlsTarget())
      .to(node_position, 1000)
      .easing(TWEEN.Easing.Quadratic.InOut)
      .start()

    t2 = new TWEEN.Tween(@graph.getCameraPosition())
      .to(target_position, 1000)
      .easing(TWEEN.Easing.Quadratic.InOut)
      .start()

    Brain.animate()

  @animate: ->
    window.requestAnimationFrame(Brain.animate);
    TWEEN.update();


module.exports = Brain

class Object3d
  @agent: (name) ->
    sphere = @sphere('#3bb8d7', 4)
    text = @sphereTitle(name)
    sphere.add(text)
    sphere.scale.set(1, 1, 1)
    return sphere

  @intent: (name) ->
    sphere = @sphere('#eee', 4)
    text = @sphereTitle(name)
    sphere.add(text)
    sphere.scale.set(0.5, 0.5, 0.5)
    return sphere

  @sphere: (color, size) ->
    geometry = new THREE.SphereGeometry(size)
    material = new THREE.MeshLambertMaterial({ color: color })
    sphere   = new THREE.Mesh(geometry, material)
    return sphere

  @sphereTitle: (text) ->
    canvas = document.createElement('canvas')
    canvas.width  = 512
    canvas.height = 512
    context = canvas.getContext('2d')

    if false
      context.fillStyle = "rgba(255, 255, 255, .25)"
      context.fillRect(0, 0, canvas.width, canvas.height)

    context.font = "45px Roboto"
    context.fillStyle = "#fff"

    metrics = context.measureText(text)
    context.fillText(text, canvas.width / 2 - metrics.width / 2, 130)

    texture = new THREE.Texture(canvas)
    texture.needsUpdate = true
    spriteMaterial = new THREE.SpriteMaterial({ map: texture })
    sprite = new THREE.Sprite(spriteMaterial)
    sprite.position.add(new THREE.Vector3(0, 0, 0))
    sprite.scale.set(30,30,30)

    return sprite;
