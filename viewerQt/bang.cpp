#include "bang.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <QTimer>

class Bang::BangCallback : public osg::NodeCallback
{
public:
  void operator()(osg::Node* nd, osg::NodeVisitor* ndv) override
  {
    Bang* bng = static_cast<Bang*>(nd);
    bng->AnimateBang(ndv->getFrameStamp()->getSimulationTime());
    traverse(nd, ndv);
  }
};

// constructor
Bang::Bang( int x, 
            int y, 
            int z, 
            std::list<osg::Node*>& toDelete) : 
  _geom(new osg::Geometry),
  _toDelete(toDelete),
  _timer(-1),
  _animateStage(0)
{
  setDataVariance(osg::Object::DYNAMIC);
  setUpdateCallback(new BangCallback);

  osg::Matrix m;
  m.makeTranslate(x, y, z);
  setMatrix(m);

  _geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
  color->push_back(osg::Vec4(1.f, 1.f, 1.f, 1.f));
  _geom->setColorArray(color.get());

  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
  vertices->push_back(osg::Vec3(-8, 0, -8)); // 1
  vertices->push_back(osg::Vec3( 8, 0, -8)); // 2
  vertices->push_back(osg::Vec3( 8, 0,  8)); // 3
  vertices->push_back(osg::Vec3(-8, 0,  8)); // 4

  _geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

  _geom->setVertexArray(vertices.get());

  osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
  normals->setBinding(osg::Array::BIND_OVERALL);
  normals->push_back(osg::Vec3(0, -1, 0));

  _geom->setNormalArray(normals.get());

  osg::ref_ptr<osg::Vec2Array> texCoord = new osg::Vec2Array;
  texCoord->push_back(osg::Vec2(2. / 3, 0));
  texCoord->push_back(osg::Vec2(     1, 0));
  texCoord->push_back(osg::Vec2(     1, 1));
  texCoord->push_back(osg::Vec2(2. / 3, 1));

  _geom->setTexCoordArray(0, texCoord.get(), osg::Array::Binding::BIND_PER_VERTEX);

  osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
  texture->setImage(osgDB::readImageFile("./Resources/bang.png"));
  texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
  texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
  osg::StateSet* state = _geom->getOrCreateStateSet();
  state->setTextureAttributeAndModes(0, texture);
  state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

  osg::ref_ptr<osg::Geode> geode = new osg::Geode;
  geode->addDrawable(_geom.get());
  addChild(geode.get());
}

Bang::Bang( const Bang& bang) : 
  osg::MatrixTransform(bang, osg::CopyOp::DEEP_COPY_ALL),
  _toDelete(bang._toDelete),
  _geom(new osg::Geometry(*bang._geom, osg::CopyOp::DEEP_COPY_ALL))
{}

void Bang::AnimateBang(double simTime)
{
  if (_timer < 0) _timer = simTime; // initialize _timer for the first time
  else if (simTime - _timer > 0.5) // if 0.5sec is elapsed after previous stage
  {
    _timer = simTime; // set new time

    if (_animateStage == 0)
    {
      _animateStage++;

      osg::ref_ptr<osg::Vec2Array> texCoord = new osg::Vec2Array;

      texCoord->push_back(osg::Vec2(1. / 3, 0));
      texCoord->push_back(osg::Vec2(2. / 3, 0));
      texCoord->push_back(osg::Vec2(2. / 3, 1));
      texCoord->push_back(osg::Vec2(1. / 3, 1));

      _geom->setTexCoordArray(0, texCoord.get(), osg::Array::Binding::BIND_PER_VERTEX);
    }
    else if (_animateStage == 1)
    {
      _animateStage++;

      osg::ref_ptr<osg::Vec2Array> texCoord = new osg::Vec2Array;

      texCoord->push_back(osg::Vec2(     0, 0));
      texCoord->push_back(osg::Vec2(1. / 3, 0));
      texCoord->push_back(osg::Vec2(1. / 3, 1));
      texCoord->push_back(osg::Vec2(     0, 1));

      _geom->setTexCoordArray(0, texCoord.get(), osg::Array::Binding::BIND_PER_VERTEX);
    }
    else if (_animateStage == 2)
    {
      _animateStage++;

      _toDelete.push_back(this);
    }
  }
}