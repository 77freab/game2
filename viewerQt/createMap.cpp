//#include "tile.h"
//#include <QXmlStreamReader>
//#include <QFile>
//#include <QMessageBox>
//
//void skipUnknownElement(QXmlStreamReader& reader)
//{
//  reader.readNext();
//  while (!reader.atEnd())
//  {
//    if (reader.isEndElement())
//    {
//      QString aa = reader.name().toString();
//      bool a = false;
//      reader.readNext();
//      break;
//    }
//    if (reader.isStartElement())
//    {
//      QString aa = reader.name().toString();
//      bool a = false;
//      if (reader.name() == "x" || reader.name() == "z")
//        skipUnknownElement(reader);
//      if (reader.name() == "tile")
//        break;
//    }
//    else
//    {
//      reader.readNext();
//    }
//  }
//}
//
//osg::Vec2i createMap(osg::ref_ptr<osg::Group> scene, 
//  std::map<osg::Vec2i, blockType>& typeMap,
//  std::map<osg::Vec2i, osg::MatrixTransform*>& tileMap, QString fileName)
//{
//  osg::Vec2i mapSize;
//  QXmlStreamReader reader;
//  QFile file(fileName);
//  file.open(QIODevice::ReadOnly | QFile::Text);
//  reader.setDevice(&file);
//  reader.readNext();
//  while (!reader.atEnd())
//  {
//    if (reader.isStartElement())
//    {
//      QString aa = reader.name().toString();
//      bool a = false;
//      if (reader.name() == "map")
//      {
//        // читаем карту
//        reader.readNext();
//        while (!reader.atEnd())
//        {
//          if (reader.isEndElement())
//          {
//            QString aa = reader.name().toString();
//            bool a = false;
//            reader.readNext();
//          }
//          if (reader.isStartElement())
//          {
//            QString aa = reader.name().toString();
//            bool a = false;
//            if (reader.name() == "sizeX")
//            {
//              mapSize[0] = (reader.readElementText().toInt() + 48) / 8;
//            }
//            else if (reader.name() == "sizeZ")
//            {
//              mapSize[1] = (reader.readElementText().toInt() + 32) / 8;
//            }
//            else if (reader.name() == "tile")
//            {
//              blockType type;
//              int x, z;
//              QString attrStr = reader.attributes()[0].name().toString();
//              if (attrStr == "type")
//              {
//                QString attrValue = reader.attributes()[0].value().toString();
//                if (attrValue == "BORDER") type = blockType::BORDER;
//                else if (attrValue == "BRICK") type = blockType::BRICK;
//                else if (attrValue == "ARMOR") type = blockType::ARMOR;
//                else if (attrValue == "WATER") type = blockType::WATER;
//                else if (attrValue == "BUSHES") type = blockType::BUSHES;
//                else if (attrValue == "ICE") type = blockType::ICE;
//              }
//              reader.readNext();
//              while (!reader.atEnd())
//              {
//                if (reader.isEndElement())
//                {
//                  QString aa = reader.name().toString();
//                  bool a = false;
//                  if (reader.name() == "tile")
//                    break;
//                  reader.readNext();
//                }
//                if (reader.isStartElement())
//                {
//                  QString aa = reader.name().toString();
//                  bool a = false;
//                  if (reader.name() == "x")
//                  {
//                    x = reader.readElementText().toInt() / 8;
//                  }
//                  else if (reader.name() == "z")
//                  {
//                    z = reader.readElementText().toInt() / 8;
//                  }
//                  else
//                  {
//                    skipUnknownElement(reader);
//                  }
//                }
//                else
//                {
//                  reader.readNext();
//                }
//              }
//              typeMap[{x, z}] = type;
//            }
//            else
//            {
//              skipUnknownElement(reader);
//            }
//          }
//          else
//          {
//            reader.readNext();
//          }
//        }
//      }
//      else
//      {
//        // херовый файл
//      }
//    }
//    else
//    {
//      reader.readNext();
//    }
//  }
//  file.close();
//
//  init();
//
//  std::map<osg::Vec2i, blockType>::const_iterator it;
//  for (int x = 0; x < mapSize[0]; x++)
//    for (int z = 0; z < mapSize[1]; z++)
//      if ((it = typeMap.find({ x, z })) != typeMap.end())
//      {
//        if ((*it).second == blockType::WATER || (*it).second == blockType::ICE)
//          tileMap[{ x, z }] = getTile(x * 8, 0, z * 8, (*it).second, true);
//        else
//          tileMap[{ x, z }] = getTile(x * 8, 0, z * 8, (*it).second);
//        scene->addChild(tileMap[{ x, z }]);
//      }
//  return mapSize;
//}