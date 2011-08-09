/*
 * Copyright © 2010-2011 Linaro Limited
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Alexandros Frantzis (glmark2)
 */
#include "scene.h"
#include "mat.h"
#include "stack.h"
#include "vec.h"
#include "log.h"
#include "shader-source.h"

#include <cmath>
#include <sstream>

static const std::string shader_file_base(GLMARK_DATA_PATH"/shaders/conditionals");

static const std::string vtx_file(shader_file_base + ".vert");
static const std::string frg_file(shader_file_base + ".frag");
static const std::string step_conditional_file(shader_file_base + "-step-conditional.all");
static const std::string step_simple_file(shader_file_base + "-step-simple.all");

SceneConditionals::SceneConditionals(Canvas &pCanvas) :
    SceneGrid(pCanvas, "conditionals")
{
    mOptions["fragment-steps"] = Scene::Option("fragment-steps", "1",
            "The number of computational steps in the fragment shader");
    mOptions["fragment-conditionals"] = Scene::Option("fragment-conditionals", "true",
            "Whether each computational step includes an if-else clause");
    mOptions["vertex-steps"] = Scene::Option("vertex-steps", "1",
            "The number of computational steps in the vertex shader");
    mOptions["vertex-conditionals"] = Scene::Option("vertex-conditionals", "true",
            "Whether each computational step includes an if-else clause");
}

SceneConditionals::~SceneConditionals()
{
}

static std::string
get_vertex_shader_source(int steps, bool conditionals)
{
    ShaderSource source(vtx_file);
    ShaderSource source_main;

    for (int i = 0; i < steps; i++) {
        if (conditionals)
            source_main.append_file(step_conditional_file);
        else
            source_main.append_file(step_simple_file);
    }

    source.replace("$MAIN$", source_main.str());

    return source.str();
}

static std::string
get_fragment_shader_source(int steps, bool conditionals)
{
    ShaderSource source(frg_file);
    ShaderSource source_main;

    for (int i = 0; i < steps; i++) {
        if (conditionals)
            source_main.append_file(step_conditional_file);
        else
            source_main.append_file(step_simple_file);
    }

    source.replace("$MAIN$", source_main.str());

    return source.str();
}

void SceneConditionals::setup()
{
    SceneGrid::setup();

    /* Parse options */
    bool vtx_conditionals = mOptions["vertex-conditionals"].value == "true";
    bool frg_conditionals = mOptions["fragment-conditionals"].value == "true";
    int vtx_steps = 0;
    int frg_steps = 0;

    std::stringstream ss;

    ss << mOptions["vertex-steps"].value;
    ss >> vtx_steps;
    ss.clear();
    ss << mOptions["fragment-steps"].value;
    ss >> frg_steps;

    /* Load shaders */
    std::string vtx_shader(get_vertex_shader_source(vtx_steps, vtx_conditionals));
    std::string frg_shader(get_fragment_shader_source(frg_steps, frg_conditionals));

    if (!Scene::load_shaders_from_strings(mProgram, vtx_shader, frg_shader))
        return;

    mProgram.start();

    std::vector<GLint> attrib_locations;
    attrib_locations.push_back(mProgram.getAttribIndex("position"));
    mMesh.set_attrib_locations(attrib_locations);

    mRunning = true;
    mStartTime = Scene::get_timestamp_us() / 1000000.0;
    mLastUpdateTime = mStartTime;
}