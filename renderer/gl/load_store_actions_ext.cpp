/*
 * Copyright 2023 Rive
 */

#include "rive/pls/gl/load_store_actions_ext.hpp"

#include "gl/gl_utils.hpp"
#include <sstream>

#include "../out/obj/generated/glsl.exports.h"
#include "../out/obj/generated/pls_load_store_ext.glsl.hpp"

namespace rive::pls
{
LoadStoreActionsEXT BuildLoadActionsEXT(const PLSRenderContext::FlushDescriptor& desc,
                                        std::array<float, 4>* clearColor4f)
{
    LoadStoreActionsEXT actions = LoadStoreActionsEXT::clearCoverage;
    if (desc.loadAction == LoadAction::clear)
    {
        UnpackColorToRGBA32F(desc.clearColor, clearColor4f->data());
        actions |= LoadStoreActionsEXT::clearColor;
    }
    else
    {
        actions |= LoadStoreActionsEXT::loadColor;
    }
    if (desc.needsClipBuffer)
    {
        actions |= LoadStoreActionsEXT::clearClip;
    }
    return actions;
}

std::ostream& BuildLoadStoreEXTGLSL(std::ostream& shader, LoadStoreActionsEXT actions)
{
    auto addDefine = [&shader](const char* name) { shader << "#define " << name << "\n"; };
    if (actions & LoadStoreActionsEXT::clearColor)
    {
        addDefine(GLSL_CLEAR_COLOR);
    }
    if (actions & LoadStoreActionsEXT::loadColor)
    {
        addDefine(GLSL_LOAD_COLOR);
    }
    if (actions & LoadStoreActionsEXT::storeColor)
    {
        addDefine(GLSL_STORE_COLOR);
    }
    if (actions & LoadStoreActionsEXT::clearCoverage)
    {
        addDefine(GLSL_CLEAR_COVERAGE);
    }
    if (actions & LoadStoreActionsEXT::clearClip)
    {
        addDefine(GLSL_CLEAR_CLIP);
    }
    shader << pls::glsl::pls_load_store_ext;
    return shader;
}
} // namespace rive::pls
