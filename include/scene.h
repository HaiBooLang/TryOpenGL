#ifndef SCENE_H
#define SCENE_H

#include <model.h>
#include <vector>
#include <map>

class Scene
{
public:
	void add(Model model, Shader shader);
	
	void draw() {
		for (auto& [model, shader] : scene)
		{
			model.Draw(shader);
		}
	}
private:
	std::map<Model, Shader> scene;
};

#endif // !SCENE_H
