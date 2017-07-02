-- Simple Scene:
-- An extremely simple scene that will render right out of the box
-- with the provided skeleton code.  It doesn't rely on hierarchical
-- transformations.

-- Create the top level root node named 'root'.
rootNode = gr.node('root')

--draw cubes
for i = 1,5 do
    cube = gr.mesh('cube', 'Lcube'..tostring(i))
    cube:scale(1.0, 1.0, 1.0)
    cube:translate(-0.5, 0.0, 0.0)
    cube:set_material(gr.material({0.9, 0.9, 0.9}, {0.8, 0.8, 0.8}, 10.0))
    rootNode:add_child(cube)
end

for i = 1,5 do
    cube = gr.mesh('cube', 'Rcube'..tostring(i))
    cube:scale(1.0, 1.0, 1.0)
    cube:translate(0.5, 0.0, 0.0)
    cube:set_material(gr.material({0.9, 0.9, 0.9}, {0.8, 0.8, 0.8}, 10.0))
    rootNode:add_child(cube)
end


-- Return the root with all of it's childern.  The SceneNode Project::m_rootNode will be set
-- equal to the return value from this Lua script.
return rootNode