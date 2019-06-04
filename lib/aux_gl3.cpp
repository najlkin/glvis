// Copyright (c) 2010, Lawrence Livermore National Security, LLC. Produced at
// the Lawrence Livermore National Laboratory. LLNL-CODE-443271. All Rights
// reserved. See file COPYRIGHT for details.
//
// This file is part of the GLVis visualization tool and library. For more
// information and source code availability see http://glvis.org.
//
// GLVis is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License (as published by the Free
// Software Foundation) version 2.1 dated February 1999.

#include "aux_gl3.hpp"
#include "aux_vis.hpp"
#include "openglvis.hpp"
#include <iostream>
#include <cstddef>

using namespace gl3;

void TextBuffer::getObjectSize(const std::string& text, int& w, int& h) {
    float x = 0.f;
    w = 0.f, h = 0.f;
    for (char c : text) {
        GlVisFont::glyph g = GetFont()->GetTexChar(c);
        float cur_x = x + g.bear_x;
        x += g.adv_x;
        if (!g.w || !g.h) {
            continue;
        }
        w = (int)(cur_x + g.w);
        h = std::max(h, (int)g.h);
    }
}

void GlDrawable::addCone(float x, float y, float z,
                         float vx, float vy, float vz,
                         float cone_scale) {
    double rhos  = sqrt (vx*vx+vy*vy+vz*vz);
    float phi   = acos(vz/rhos);
    float theta = atan2 (vy, vx);

    glm::mat4 mtx(1.0);
    mtx = glm::translate(mtx, glm::vec3(x, y, z));
    mtx = glm::scale(mtx, glm::vec3(cone_scale));
    mtx = glm::translate(mtx, glm::vec3(x, y, z));
    mtx = glm::rotate(mtx, theta, glm::vec3(0.f, 0.f, 1.f));
    mtx = glm::rotate(mtx, phi, glm::vec3(0.f, 1.f, 0.f));
    glm::mat3 norm(mtx);
    norm = glm::inverseTranspose(norm);

    glm::vec3 start_vtx = glm::vec3(mtx * glm::vec4(0.f, 0.f, 0.f, 1.f));
    glm::vec3 start_norm = glm::vec3(norm * glm::vec3(0.f, 0.f, 1.f));

    glm::vec3 base_pts[] = {
        glm::vec3(mtx * glm::vec4(1, 0, -4, 1)),
        glm::vec3(mtx * glm::vec4(cos(2*M_PI/4), sin(2*M_PI/4), -4, 1)),
        glm::vec3(mtx * glm::vec4(cos(4*M_PI/4), sin(4*M_PI/4), -4, 1)),
        glm::vec3(mtx * glm::vec4(cos(6*M_PI/4), sin(6*M_PI/4), -4, 1)),
    };

    float nz = (1.0/4.0);
    glm::vec3 base_norms[] = {
        glm::vec3(norm * glm::vec3(1, 0, nz)),
        glm::vec3(norm * glm::vec3(cos(2*M_PI/4), sin(2*M_PI/4), nz)),
        glm::vec3(norm * glm::vec3(cos(4*M_PI/4), sin(4*M_PI/4), nz)),
        glm::vec3(norm * glm::vec3(cos(6*M_PI/4), sin(6*M_PI/4), nz)),
    };
    
    float* orig = glm::value_ptr(start_vtx);
    float* orig_n = glm::value_ptr(start_norm);
    float* base[4] = {
        glm::value_ptr(base_pts[0]),
        glm::value_ptr(base_pts[1]),
        glm::value_ptr(base_pts[2]),
        glm::value_ptr(base_pts[3])
    };
    float* base_n[4] = {
        glm::value_ptr(base_norms[0]),
        glm::value_ptr(base_norms[1]),
        glm::value_ptr(base_norms[2]),
        glm::value_ptr(base_norms[3])
    };

    std::vector<float> cone_pts;
    for (int i = 0; i < 4; i++) {
        addTriangle(
            VertexNorm {
                {orig[0],   orig[1],   orig[2]},
                {orig_n[0], orig_n[1], orig_n[2]}
            },
            VertexNorm {
                {base[i][0],   base[i][1],   base[i][2]},
                {base_n[i][0], base_n[i][1], base_n[i][2]}
            },
            VertexNorm {
                {base[(i+1)%4][0],   base[(i+1)%4][1],   base[(i+1)%4][2]},
                {base_n[(i+1)%4][0], base_n[(i+1)%4][1], base_n[(i+1)%4][2]}
            }
        );
    }
}

void GlBuilder::saveVertex(const GlBuilder::_vertex& v) {
    GLenum dst_buf = is_line ? GL_LINES : GL_TRIANGLES;
    if (!use_norm) {
        if (use_color) {
            parent_buf->getBuffer<VertexColor>(dst_buf)
                      ->addVertex(VertexColor{v.coords, v.color});
        } else if (use_tex) {
            parent_buf->getBuffer<VertexTex>(dst_buf)
                      ->addVertex(VertexTex{v.coords, v.texcoord});
        } else {
            parent_buf->getBuffer<Vertex>(dst_buf)
                      ->addVertex(Vertex{v.coords});
        }
    } else {
        if (use_color) {
            parent_buf->getBuffer<VertexNormColor>(dst_buf)
                      ->addVertex(VertexNormColor{v.coords, v.norm, v.color});
        } else if (use_tex) {
            parent_buf->getBuffer<VertexNormTex>(dst_buf)
                      ->addVertex(VertexNormTex{v.coords, v.norm, v.texcoord});
        } else {
            parent_buf->getBuffer<VertexNorm>(dst_buf)
                      ->addVertex(VertexNorm{v.coords, v.norm});
        }
    }
}
