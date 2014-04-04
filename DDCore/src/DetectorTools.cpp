// $Id: Readout.cpp 985 2014-01-30 13:50:10Z markus.frank@cern.ch $
//====================================================================
//  AIDA Detector description implementation
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================
// Framework include files
#include "DD4hep/DetectorTools.h"
#include "DD4hep/Printout.h"
#include "DD4hep/LCDD.h"
#include <stdexcept>
#include <memory>

// ROOT include files
#include "TGeoMatrix.h"

using namespace std;
using namespace DD4hep;
using namespace DD4hep::Geometry;

/// Collect detector elements to any parent detector element
bool DetectorTools::findParent(DetElement parent, DetElement elt, vector<DetElement>& detectors)  {
  detectors.clear();
  if ( parent.isValid() && elt.isValid() )  {
    if ( parent.ptr() == elt.ptr() )  {
      detectors.push_back(elt);
      return true;
    }
    ElementPath elements;
    for(DetElement par = elt; par.isValid(); par = par.parent())  {
      elements.push_back(par);
      if ( par.ptr() == parent.ptr() )  {
	detectors = elements;
	return true;
      }
    }
  }
  throw runtime_error("Search for parent detector element with invalid handles not allowed.");
}

/// Find Child of PlacedVolume and assemble on the fly the path of PlacedVolumes
bool DetectorTools::findChild(PlacedVolume parent, PlacedVolume child, PlacementPath& path) {
  if ( parent.isValid() && child.isValid() ) {
    // Check self
    if ( parent.ptr() == child.ptr() ) {
      path.push_back(child);
      return true;
    }    
    TIter next(parent->GetVolume()->GetNodes());
    // Now check next layer children
    for (TGeoNode *daughter = (TGeoNode*) next(); daughter; daughter = (TGeoNode*) next()) {
      if ( daughter == child.ptr() ) {
        path.push_back(daughter);
        return true;
      }
    }
    next.Reset();
    // Finally crawl down the tree
    for (TGeoNode *daughter = (TGeoNode*) next(); daughter; daughter = (TGeoNode*) next()) {
      PlacementPath sub_path;
      bool res = findChild(daughter, child, sub_path);
      if (res) {
	path.insert(path.end(), sub_path.begin(), sub_path.end());
        path.push_back(daughter);
        return res;
      }
    }
  }
  return false;
}

/// Find Child of PlacedVolume and assemble on the fly the path of PlacedVolumes
static bool findChildByName(PlacedVolume parent, PlacedVolume child, DetectorTools::PlacementPath& path) {
  if ( parent.isValid() && child.isValid() ) {
    // Check self
    if ( 0 == ::strcmp(parent.ptr()->GetName(),child.ptr()->GetName()) ) {
      path.push_back(child);
      return true;
    }    
    TIter next(parent->GetVolume()->GetNodes());
    // Now check next layer children
    for (TGeoNode *daughter = (TGeoNode*) next(); daughter; daughter = (TGeoNode*) next()) {
      if ( 0 == ::strcmp(daughter->GetName(),child.ptr()->GetName()) ) {
        path.push_back(daughter);
        return true;
      }
    }
    next.Reset();
    // Finally crawl down the tree
    for (TGeoNode *daughter = (TGeoNode*) next(); daughter; daughter = (TGeoNode*) next()) {
      DetectorTools::PlacementPath sub_path;
      bool res = findChildByName(daughter, child, sub_path);
      if (res) {
	path.insert(path.end(), sub_path.begin(), sub_path.end());
        path.push_back(daughter);
        return res;
      }
    }
  }
  return false;
}

/// Collect detector elements to the top detector element (world)
void DetectorTools::elementPath(DetElement elt, ElementPath& detectors) {
  for(DetElement par = elt; par.isValid(); par = par.parent())
    detectors.push_back(par);
}

/// Collect detector elements to any parent detector element
void DetectorTools::elementPath(DetElement parent, DetElement elt, ElementPath& detectors)  {
  if ( findParent(parent,elt,detectors) ) 
    return;
  throw runtime_error(string("The detector element ")+parent.name()+string(" is no parent of ")+elt.name());
}

/// Collect detector elements placements to the top detector element (world) [fast, but may have holes!]
void DetectorTools::elementPath(DetElement element, vector<PlacedVolume>& det_nodes) {
  for(DetElement par = element; par.isValid(); par = par.parent())  {
    PlacedVolume pv = par.placement();
    if ( pv.isValid() ) {
      det_nodes.push_back(pv);
    }
  }
}

/// Assemble the path of the PlacedVolume selection
std::string DetectorTools::elementPath(const ElementPath& nodes)  {
  string s = "";
  for(ElementPath::const_reverse_iterator i=nodes.rbegin();i!=nodes.rend();++i)
    s += "/" + string((*i)->GetName());
  return s;
}

/// Assemble the path of a particular detector element
std::string DetectorTools::elementPath(DetElement element)  {
  ElementPath nodes;
  elementPath(element,nodes);
  return elementPath(nodes);
}

/// Find DetElement as child of the top level volume by it's absolute path
DetElement DetectorTools::findElement(LCDD& lcdd, const std::string& path)   {
  return findElement(lcdd.world(),path);
}

/// Find DetElement as child of a parent by it's relative or absolute path
DetElement DetectorTools::findElement(DetElement parent, const std::string& subpath)  {
  if ( parent.isValid() )   {
    size_t idx = subpath.find('/',1);
    if ( subpath[0] == '/' )  {
      DetElement top = topElement(parent);
      if ( idx == string::npos ) return top;
      return findElement(top,subpath.substr(idx+1));
    }
    if ( idx == string::npos )
      return parent.child(subpath);
    string name = subpath.substr(0,idx);
    DetElement node = parent.child(name);
    if ( node.isValid() )  {
      return findElement(node,subpath.substr(idx+1));
    }
    throw runtime_error("DD4hep: DetElement "+parent.path()+" has no child named:"+name+" [No such child]");
  }
  throw runtime_error("DD4hep: Cannot determine child with path "+subpath+" from invalid parent [invalid handle]");
}

/// Determine top level element (=world) for any element walking up the detector element tree
DetElement DetectorTools::topElement(DetElement child)   {
  if ( child.isValid() )   {
    if ( child.parent().isValid() )
      return topElement(child.parent());
    return child;
  }
  throw runtime_error("DD4hep: DetElement cannot determine top parent (world) [invalid handle]");
}

/// Collect detector elements placements to the top detector element (world) [no holes!]
void DetectorTools::placementPath(DetElement element, PlacementPath& nodes)   {
  PlacementPath det_nodes, all_nodes;
  DetectorTools::elementPath(element,det_nodes);
  for (size_t i = 0, n = det_nodes.size(); i < n-1; ++i)  {
    if (!findChildByName(det_nodes[i + 1], det_nodes[i], nodes))  {
      throw runtime_error("DD4hep: DetElement cannot determine placement path of " 
			  + string(element.name()) + " [internal error]");
    }
  }
  if ( det_nodes.size()>0 )   {
    nodes.push_back(det_nodes.back());
  }
}

/// Collect detector elements placements to the parent detector element [no holes!]
void DetectorTools::placementPath(DetElement parent, DetElement child, PlacementPath& nodes)   {
  ElementPath   det_nodes;
  PlacementPath all_nodes;
  DetectorTools::elementPath(parent,child,det_nodes);
  for (size_t i = 0, n = det_nodes.size(); i < n-1; ++i) {
    if (!findChild(det_nodes[i + 1], det_nodes[i], nodes)) {
      throw runtime_error("Invalid placement path "+elementPath(det_nodes)+" [internal error]");
    }
  }
  if ( det_nodes.size()>0 ) nodes.push_back(det_nodes.back());
}

/// Assemble the path of the PlacedVolume selection
std::string DetectorTools::placementPath(DetElement element)  {
  PlacementPath path;
  placementPath(element,path);
  return placementPath(path);
}

/// Assemble the path of the PlacedVolume selection
std::string DetectorTools::placementPath(const PlacementPath& nodes)  {
  string s="";
  for(PlacementPath::const_reverse_iterator i=nodes.rbegin();i!=nodes.rend();++i)
    s += "/" + string((*i)->GetName());
  return s;
}

/// Create cached matrix to transform to positions to an upper level Placement
TGeoMatrix* DetectorTools::placementTrafo(const PlacementPath& nodes, bool inverse) {
  if (nodes.size() < 2) {
    return new TGeoHMatrix(*gGeoIdentity);
  }
  auto_ptr<TGeoHMatrix> mat(new TGeoHMatrix(*gGeoIdentity));
  for (size_t i = 0, n=nodes.size(); n>0 && i < n-1; ++i)  {
    const PlacedVolume& p = nodes[i];
    TGeoMatrix* m = p->GetMatrix();
    mat->MultiplyLeft(m);
  }
  if ( inverse )  {
    auto_ptr<TGeoHMatrix> inv(new TGeoHMatrix(mat->Inverse()));
    mat = inv;
  }
  return mat.release();
}

