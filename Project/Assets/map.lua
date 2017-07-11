-- Simple Scene:
-- An extremely simple scene that will render right out of the box
-- with the provided skeleton code.  It doesn't rely on hierarchical
-- transformations.

-- Create the top level root node named 'root'.
rootNode = gr.node('root')

blue = gr.material({0.7, 0.6, 1}, {0.5, 0.4, 0.8}, 25)
white = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0)
grey = gr.material({0.5, 0.5, 0.5}, {0.5,0.5,0.5}, 100.0)
black = gr.material({0.2, 0.2, 0.2}, {0.1,0.1,0.1}, 10)
orange = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 10)

bcube = gr.mesh('cube', 'bcube', 1, 1,1)
bcube:scale(1.0, 1.0, 1.0)
bcube:translate(-0.5, 0.0, 10.0)
bcube:set_material(gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0))
rootNode:add_child(bcube)

bcube = gr.mesh('cube', 'bcube', 1, 1,1)
bcube:scale(1.0, 1.0, 1.0)
bcube:translate(0.5, 0.0, 9.0)
bcube:set_material(gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0))
rootNode:add_child(bcube)

-- | |
for i = 1,7 do
    cube = gr.mesh('cube', 'Lcube'..tostring(i), 1, 1,1)
    cube:scale(1.0, 1.0, 1.0)
    cube:translate(-1.5, 0.0, 9.0 - i - i*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

for i = 1,7 do
    cube = gr.mesh('cube', 'Rcube'..tostring(i), 1, 1,1)
    cube:scale(1.0, 1.0, 1.0)
    cube:translate(1.8, 0.0, 9.0 - i - i*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

-- __   __
--   | |
for i = 1,7 do
    cube = gr.mesh('cube', 'LBcube'..tostring(i), 1, 1,1)
    cube:translate(-1.5 - i - i*0.1, 0.0, 9.0 - 7 - 7*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

for i = 1,7 do
    cube = gr.mesh('cube', 'RBcube'..tostring(i), 1, 1,1)
    cube:translate(1.8 + i + i*0.1, 0.0, 9.0 - 7 - 7*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

--|       |
--|__   __|
--   | |
for i = 1,20 do
    cube = gr.mesh('cube', 'LLcube'..tostring(i), 1, 1,1)
    cube:translate(-9.2, 0.0, 1.3 - i - i*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

for i = 1,20 do
    cube = gr.mesh('cube', 'RRcube'..tostring(i), 1,1,1)
    cube:translate(9.5, 0.0, 1.3 - i - i*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

-- _______
--|       |
--|__   __|
--   | |
for i = 1,18 do
    cube = gr.mesh('cube', 'Tcube'..tostring(i),1,1,1)
    cube:translate(-9.2 + (i -1) + (i-1)*0.1, 0.0, 1.3 - 21 - 21*0.1)
    cube:set_material(white)
    rootNode:add_child(cube)
end

obstacle = gr.mesh('cube', 't2', 1, 1,1)
obstacle:scale(1.0, 1.0, 1.0)
obstacle:translate(-2, 0.0, -5)
obstacle:set_material(gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0))
rootNode:add_child(obstacle)

obstacle2 = gr.mesh('cube', 't3', 1, 1,1)
obstacle2:scale(1.0, 1.0, 1.0)
obstacle2:translate(2, 0.0, -5)
obstacle2:set_material(gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 100.0))
rootNode:add_child(obstacle2)

plane = gr.mesh('plane', 'plane',2.0, 0.1, 2.0)
plane:scale(12, 1.0, 20)
plane:translate(0.0, -0.5, -5.0)
plane:set_material(white)
rootNode:add_child(plane)

player = gr.mesh('player', 'player', 0.5, 0.1, 1.0)
player:scale(1.0, 1.0, 1.0)
player:translate(0.2, 0.3, 7.6)
player:set_material(black)
rootNode:add_child(player)

enemy1 = gr.mesh('sphere', 'e1', 0.9, 0.9, 0.9)
enemy1:scale(0.7,0.7,0.7)
enemy1:translate(5, 0.2, -15)
enemy1:set_material(black)
rootNode:add_child(enemy1)

enemy2 = gr.mesh('sphere', 'e2',0.9, 0.9, 0.9)
enemy2:scale(0.7, 0.7, 0.7)
enemy2:translate(-5, 0.2, -15)
enemy2:set_material(black)
rootNode:add_child(enemy2)

t1 = gr.mesh('cube', 't1', 1, 1,1)
t1:scale(1.0, 1.0, 1.0)
t1:translate(-5, 0.0, -3)
t1:set_material(blue)
rootNode:add_child(t1)

r1 = gr.mesh('cube', 'r1', 1, 1,1)
r1:scale(1.0, 1.0, 1.0)
r1:translate(5, 0.0, -3)
r1:set_material(white)
rootNode:add_child(r1)

--t1 = gr.mesh('sphere', 't2', 1.5, 1.5,1.5)
--t1:scale(1.5, 1.5, 1.5)
--t1:translate(5, 3, 3)
--t1:set_material(orange)
--rootNode:add_child(t1)

-- Return the root with all of it's childern.  The SceneNode Project::m_rootNode will be set
-- equal to the return value from this Lua script.
return rootNode