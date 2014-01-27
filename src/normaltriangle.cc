
#include "normaltriangle.hh"

void NormalTriangle::updateNormals(std::map<Vec3d, std::list<Triangle*>>& ptMap)
{
  no1_ = this->updateVertex(ptMap[pt1_]);
  no2_ = this->updateVertex(ptMap[pt2_]);
  no3_ = this->updateVertex(ptMap[pt3_]);
}
