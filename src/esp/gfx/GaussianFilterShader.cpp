// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "GaussianFilterShader.h"
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/FormatStl.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/DebugTools/ColorMap.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/GL/Version.h>
#include <Magnum/ImageView.h>
#include <Magnum/PixelFormat.h>

namespace Cr = Corrade;
namespace Mn = Magnum;

static void importShaderResources() {
  CORRADE_RESOURCE_INITIALIZE(ShaderResources)
}

namespace esp {
namespace gfx {

enum {
  SourceTextureUnit = 1,
};

GaussianFilterShader::GaussianFilterShader() {
  if (!Corrade::Utility::Resource::hasGroup("default-shaders")) {
    importShaderResources();
  }

  const Corrade::Utility::Resource rs{"default-shaders"};

#ifdef MAGNUM_TARGET_WEBGL
  Mn::GL::Version glVersion = Mn::GL::Version::GLES300;
#else
  Mn::GL::Version glVersion = Mn::GL::Version::GL330;
#endif

  Mn::GL::Shader vert{glVersion, Mn::GL::Shader::Type::Vertex};
  Mn::GL::Shader frag{glVersion, Mn::GL::Shader::Type::Fragment};

  vert.addSource("#define OUTPUT_UV\n").addSource(rs.get("bigTriangle.vert"));

  frag.addSource("#define EXPLICIT_ATTRIB_LOCATION\n")
      .addSource(Cr::Utility::formatString(
          "#define OUTPUT_ATTRIBUTE_LOCATION_COLOR {}\n", ColorOutput))
      .addSource(rs.get("gaussianFilter.frag"));

  CORRADE_INTERNAL_ASSERT_OUTPUT(Mn::GL::Shader::compile({vert, frag}));

  attachShaders({vert, frag});

  CORRADE_INTERNAL_ASSERT_OUTPUT(link());

  // setup texture binding point
  setUniform(uniformLocation("SourceTexture"), SourceTextureUnit);

  // setup uniforms
  filterHorizontallyUniform_ = uniformLocation("FilterHorizontally");
  CORRADE_INTERNAL_ASSERT(filterHorizontallyUniform_ >= 0);
}

GaussianFilterShader& GaussianFilterShader::bindTexture(
    Magnum::GL::Texture2D& texture) {
  texture.bind(SourceTextureUnit);
  return *this;
}

GaussianFilterShader& GaussianFilterShader::setFilteringDirection(
    FilteringDirection dir) {
  if (dir == FilteringDirection::Horizontal) {
    setUniform(filterHorizontallyUniform_, 1);
  } else {
    setUniform(filterHorizontallyUniform_, 0);
  }
  return *this;
}

}  // namespace gfx
}  // namespace esp