#include "tile.h"
#include <osgDB/ReadFile>

void tile::setTexture(std::string texPath)
{
  _image = osgDB::readImageFile("./Resources/" + texPath);
  _texture->setImage(_image);
  _texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
  _texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
  osg::StateSet* state = _geom->getOrCreateStateSet();
  state->setTextureAttributeAndModes(0, _texture);
  state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
}

tile::tile(int x, int y, int z, std::string texPath, bool pr)
  : _geode(new osg::Geode), _normals(new osg::Vec3Array), _geom(new osg::Geometry),
  _vertices(new osg::Vec3Array), _color(new osg::Vec4Array),
  _texCoord(new osg::Vec2Array), _texture(new osg::Texture2D)
{
  osg::Matrix m;
  m.makeTranslate(x, y, z);
  this->setMatrix(m);

  _geom->setColorArray(_color.get());
  _geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  _color->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
  if (pr)
  {
    _geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

    _vertices->push_back(osg::Vec3(0, 0, 0)); // 1
    _vertices->push_back(osg::Vec3(8, 0, 0)); // 2
    _vertices->push_back(osg::Vec3(8, 0, 8)); // 3
    _vertices->push_back(osg::Vec3(0, 0, 8)); // 4
  }
  else
  {

    //_geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, 12));

    //_vertices->push_back(osg::Vec3(0,  0, 0)); // 1
    //_vertices->push_back(osg::Vec3(8,  0, 0)); // 2
    //_vertices->push_back(osg::Vec3(8, -8, 0)); // 3
    //_vertices->push_back(osg::Vec3(8,  0, 8)); // 4
    //_vertices->push_back(osg::Vec3(8, -8, 8)); // 5
    //_vertices->push_back(osg::Vec3(0,  0, 8)); // 6
    //_vertices->push_back(osg::Vec3(0, -8, 8)); // 7
    //_vertices->push_back(osg::Vec3(0,  0, 0)); // 8
    //_vertices->push_back(osg::Vec3(0, -8, 0)); // 9
    //_vertices->push_back(osg::Vec3(8, -8, 0)); // 10
    //_vertices->push_back(osg::Vec3(0, -8, 8)); // 11
    //_vertices->push_back(osg::Vec3(8, -8, 8)); // 12

    _geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
    _geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 4, 4));
    _geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 8, 4));
    _geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 12, 4));
    _geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 16, 4));

    _vertices->push_back(osg::Vec3(0, 0, 0)); // 1
    _vertices->push_back(osg::Vec3(8, 0, 0)); // 2
    _vertices->push_back(osg::Vec3(8, -8, 0)); // 3
    _vertices->push_back(osg::Vec3(0, -8, 0)); // 4

    _vertices->push_back(osg::Vec3(8, 0, 0)); // 5
    _vertices->push_back(osg::Vec3(8, 0, 8)); // 6
    _vertices->push_back(osg::Vec3(8, -8, 8)); // 7
    _vertices->push_back(osg::Vec3(8, -8, 0)); // 8

    _vertices->push_back(osg::Vec3(8, 0, 8)); // 9
    _vertices->push_back(osg::Vec3(0, 0, 8)); // 10
    _vertices->push_back(osg::Vec3(0, -8, 8)); // 11
    _vertices->push_back(osg::Vec3(8, -8, 8)); // 12

    _vertices->push_back(osg::Vec3(0, 0, 8)); // 13
    _vertices->push_back(osg::Vec3(0, 0, 0)); // 14
    _vertices->push_back(osg::Vec3(0, -8, 0)); // 15
    _vertices->push_back(osg::Vec3(0, -8, 8)); // 16

    _vertices->push_back(osg::Vec3(0, -8, 0)); // 17
    _vertices->push_back(osg::Vec3(8, -8, 0)); // 18
    _vertices->push_back(osg::Vec3(8, -8, 8)); // 19
    _vertices->push_back(osg::Vec3(0, -8, 8)); // 20
  }

  _geom->setVertexArray(_vertices.get());

  //_normals->setBinding(osg::Array::BIND_PER_PRIMITIVE_SET);

  //_normals->push_back(osg::Vec3( 0,  0, -1));
  //_normals->push_back(osg::Vec3( 1,  0,  0));
  //_normals->push_back(osg::Vec3( 0,  0,  1));
  //_normals->push_back(osg::Vec3(-1,  0,  0));
  //_normals->push_back(osg::Vec3( 0, -1,  0));

  _normals->setBinding(osg::Array::BIND_OVERALL);
  _normals->push_back(osg::Vec3( 0, -1,  0));

  _geom->setNormalArray(_normals.get());

  _texCoord->push_back(osg::Vec2(0, 0));
  _texCoord->push_back(osg::Vec2(1, 0));
  _texCoord->push_back(osg::Vec2(1, 1));
  _texCoord->push_back(osg::Vec2(0, 1));
  if (!pr)
  {
    for (int i = 0; i < 4; ++i)
    {
      _texCoord->push_back(osg::Vec2(0, 0));
      _texCoord->push_back(osg::Vec2(1, 0));
      _texCoord->push_back(osg::Vec2(1, 1));
      _texCoord->push_back(osg::Vec2(0, 1));
    }
  }
  _geom->setTexCoordArray(0, _texCoord.get(), osg::Array::Binding::BIND_OVERALL);
  setTexture(texPath);

  _geode->addDrawable(_geom.get());
  this->addChild(_geode.get());
}