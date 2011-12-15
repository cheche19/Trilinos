//-------------------------------------------------------------------------
// Filename      : PerceptMesquiteMesh.cpp, derived from SCVFracMesquiteMesh.cpp
//
// Purpose       : mesh interface for using Mesquite 
//
// Description   : subclass of Mesquite::Mesh
//
// Creator       : Steve Kennon, derived from Steve Owen's work
//
// Creation Date : Nov 2011
//
// Owner         : Steve Kennon
//-------------------------------------------------------------------------

#ifdef STK_BUILT_IN_SIERRA
// ibm can't compile mesquite
#if !defined(__IBMCPP__)
#include "PerceptMesquiteMesh.hpp"
//#include <mesquite/MsqVertex.hpp>
#include <MsqVertex.hpp>

#include <map>
#include <algorithm>

namespace stk {
  namespace percept {


#define PRINT_ERROR(a) do { std::cout << "PerceptMesquiteMesh::ERROR: " << a << std::endl; } while (0)

    //============================================================================
    // Description: initialize data
    // Author: sjowen
    // Date: 5/19/2011
    //============================================================================
    void PerceptMesquiteMesh::init(PerceptMesh *eMesh)
    {
      m_eMesh = eMesh;
#if 0
      isSetup = false;
      mOwners = entities;
      nodeArray = NULL;
      elementArray = NULL;
      numNodes = 0;
      numElements = 0;
      triExists = false;
      quadExists = false;
      tetExists = false;
      hexExists = false;

      doJacobiIterations = false;
  
      freeScheme = false;
      jacobiCoords = NULL;
      nextJacobiCoordsIndex = 0;
  
      myDimension = 0;
#endif
    }



    //============================================================================
    // Description: constructor
    // Author: sjowen
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    PerceptMesquiteMesh::PerceptMesquiteMesh(PerceptMesh *eMesh)
    {  
      init(eMesh);
    }

    //============================================================================
    // Description: called once to set up nodes/elements to be smoothed
    // Author: sjowen
    // Note: assumes nodes have pointers to hexes and tets
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    int PerceptMesquiteMesh::setup()
    {
      return 0;
    }

    //============================================================================
    // Description: determine whether a node is at the boundary of the elementArray
    // Notes: based on whether all elements surrounding a node are in the includedElements set
    // Author: sjowen, srkennon
    // Date: 04/19/2011
    //============================================================================
    bool PerceptMesquiteMesh::is_on_my_patch_boundary(stk::mesh::Entity *node_ptr)
    {
      return false;
    }

    //============================================================================
    // Description: clean out the data to call setup again
    // Author: sjowen, srkennon
    // Date: 5/19/2011
    //============================================================================
    void PerceptMesquiteMesh::clean_out()
    {
  
    }

    //============================================================================
    // Description: destructor
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    PerceptMesquiteMesh::~PerceptMesquiteMesh()
    {
    
      clean_out();
    
    }

    //============================================================================
    // Description: Returns whether this mesh lies in a 2D or 3D coordinate system.
    // Notes: We always pass in nodes in with three coordinates.  This may change
    // in the future if we want to do smoothing in a parametric space, but
    // for now, we are always in three-dimensions.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================  
    int PerceptMesquiteMesh::get_geometric_dimension(
                                                     Mesquite::MsqError &/*err*/)
    {
      return m_eMesh->getSpatialDim();
    }

    //============================================================================
    // Description: Returns the number of verticies for the entity.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================ 
    //! Returns the number of verticies for the entity.
    size_t PerceptMesquiteMesh::get_total_vertex_count(
                                                       Mesquite::MsqError &/*err*/) const
    {
      return (size_t) m_eMesh->getNumberNodes();
    }

    //============================================================================
    // Description: Returns the number of elements for the entity.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    size_t PerceptMesquiteMesh::get_total_element_count(
                                                        Mesquite::MsqError &/*err*/) const
    {
      return (size_t) m_eMesh->getNumberElements();
    }

    //============================================================================
    // Description: Fills array with handles to all vertices in the mesh.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    void PerceptMesquiteMesh::get_all_vertices(
                                               std::vector<Mesquite::Mesh::VertexHandle> &vertices,
                                               Mesquite::MsqError &/*err*/)
    {
      //otherwise add a handle to each vertex to the given array.
      //int index = 0;
      vertices.clear();
  
      const std::vector<stk::mesh::Bucket*> & buckets = m_eMesh->getBulkData()->buckets( m_eMesh->node_rank() );

      for ( std::vector<stk::mesh::Bucket*>::const_iterator k = buckets.begin() ; k != buckets.end() ; ++k )
        {
          //if (removePartSelector(**k))
          {
            stk::mesh::Bucket & bucket = **k ;

            const unsigned num_entity_in_bucket = bucket.size();
            for (unsigned ientity = 0; ientity < num_entity_in_bucket; ientity++)
              {
                stk::mesh::Entity& node = bucket[ientity];
                vertices.push_back(reinterpret_cast<Mesquite::Mesh::VertexHandle>( &node ) );
              }
          }
        }
    }

    //============================================================================
    // Description: Fills array with handles to all elements in the mesh.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    void PerceptMesquiteMesh::get_all_elements(   
                                               std::vector<Mesquite::Mesh::ElementHandle> &elements,      
                                               Mesquite::MsqError &/*err*/ )
    {
      elements.clear();

      const std::vector<stk::mesh::Bucket*> & buckets = m_eMesh->getBulkData()->buckets( m_eMesh->element_rank() );

      for ( std::vector<stk::mesh::Bucket*>::const_iterator k = buckets.begin() ; k != buckets.end() ; ++k )
        {
          //if (removePartSelector(**k))
          {
            stk::mesh::Bucket & bucket = **k ;

            const unsigned num_entity_in_bucket = bucket.size();
            for (unsigned ientity = 0; ientity < num_entity_in_bucket; ientity++)
              {
                stk::mesh::Entity& element = bucket[ientity];
                elements.push_back(reinterpret_cast<Mesquite::Mesh::VertexHandle>( &element ) );
              }
          }
        }

    }

    //============================================================================
    // Description: Returns true or false, indicating whether the vertex
    //! is allowed to moved.
    //! Note that this is a read-only
    //! property; this flag can't be modified by users of the
    //! Mesquite::Mesh interface.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    void PerceptMesquiteMesh::vertices_get_fixed_flag(
                                                      const Mesquite::Mesh::VertexHandle vert_array[], 
                                                      std::vector<bool>& fixed_flag_array,
                                                      size_t num_vtx, 
                                                      Mesquite::MsqError &err )
    {
  
      unsigned int i;

      if(!m_boundarySelector)
        {     
          MSQ_SETERR(err)("PerceptMesquiteMesh::vertex_is_on_boundary: No selector to determine vertex fixed-ness.", Mesquite::MsqError::INVALID_STATE);
          PRINT_ERROR(" PerceptMesquiteMesh::vertex_is_on_boundary: No selector to determine vertex fixed-ness.\n");
          return;
        }
    
      for (i = 0; i < num_vtx; ++i)
        {
          stk::mesh::Entity* node_ptr = reinterpret_cast<stk::mesh::Entity *>(vert_array[i]);
    
          //if we've got a null pointer, something is wrong.
          if(node_ptr==NULL)
            {
              MSQ_SETERR(err)("PerceptMesquiteMesh::vertex_is_on_boundary: Null pointer to vertex.", Mesquite::MsqError::INVALID_STATE);
              PRINT_ERROR(" PerceptMesquiteMesh::vertex_is_on_boundary: Null pointer to vertex.\n");
              return;
            }
    
          //if the owner is something other than the top-level owner, the node
          // is on the boundary; otherwise, it isn't.
          if ((*m_boundarySelector)(*node_ptr))
            fixed_flag_array.push_back(true);
          else
            fixed_flag_array.push_back(false);
        }
   
    }


    //============================================================================
    // Description: vertex is "slaved" if it is on a higher order element or
    // has an owner that is not in the mOwner list.  - for now we don't distinguish
    // higher order nodes
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    void PerceptMesquiteMesh::vertices_get_slaved_flag( const VertexHandle vert_array[], 
                                                        std::vector<bool>& slaved_flag_array,
                                                        size_t num_vtx, 
                                                        Mesquite::MsqError &err )
    {
  
      unsigned int i;
    
      for (i = 0; i < num_vtx; ++i)
        {
          stk::mesh::Entity* node_ptr = reinterpret_cast<stk::mesh::Entity *>(vert_array[i]);
    
          //if we've got a null pointer, something is wrong.
          if(node_ptr==NULL){
            MSQ_SETERR(err)("PerceptMesquiteMesh::vertices_get_slaved_flag: Null pointer to vertex.", Mesquite::MsqError::INVALID_STATE);
            PRINT_ERROR(" PerceptMesquiteMesh::vertices_get_slaved_flag: Null pointer to vertex.\n");
            return;
          }
    
          //if the owner is something other than the top-level owner, the node
          // is on the boundary; otherwise, it isn't.
    
          slaved_flag_array.push_back(false);

        }
  
    }


    //============================================================================
    // Description: Get location of a vertex
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    void PerceptMesquiteMesh::vertices_get_coordinates(
                                                       const Mesquite::Mesh::VertexHandle vert_array[],
                                                       Mesquite::MsqVertex* coordinates,
                                                       size_t num_vtx,
                                                       Mesquite::MsqError &err)
    {
      unsigned int i;
      stk::mesh::Entity* node_ptr = NULL;
      for (i = 0; i<num_vtx; ++i){
    
        node_ptr=reinterpret_cast<stk::mesh::Entity*>(vert_array[i]);
    
        //if null pointer, there is a problem somewhere.  We set the vector's
        // position to (0,0,0) just to avoid un-initialized variable issues.
        if(node_ptr==NULL) {
          MSQ_SETERR(err)("PerceptMesquiteMesh::vertex_get_coordinates: invalid vertex handle.", Mesquite::MsqError::INVALID_STATE);
          PRINT_ERROR("PerceptMesquiteMesh::vertex_get_coordinates: invalid vertex handle.\n");
          coordinates[i].set(0.0,0.0,0.0);
          return;
        }
        //set coordinates to the vertex's position.
        stk::mesh::FieldBase* field = m_eMesh->getCoordinatesField();
        double *f_data = PerceptMesh::field_data(field, *node_ptr);

        coordinates[i].set(f_data[0], f_data[1], f_data[2]);
      }
    }


    //============================================================================
    // Description: Set the location of a vertex.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    void PerceptMesquiteMesh::vertex_set_coordinates(
                                                     VertexHandle vertex,
                                                     const Mesquite::Vector3D &coordinates,
                                                     Mesquite::MsqError &err)
    {
      stk::mesh::Entity* node_ptr = reinterpret_cast<stk::mesh::Entity*>(vertex);
    
      //if null pointer, there is a problem somewhere.  We set the vector's
      // position to (0,0,0) just to avoid un-initialized variable issues.
      if(node_ptr==NULL) {
        MSQ_SETERR(err)("PerceptMesquiteMesh::vertex_set_coordinates: invalid vertex handle.", Mesquite::MsqError::INVALID_STATE);
        PRINT_ERROR("PerceptMesquiteMesh::vertex_set_coordinates: invalid vertex handle.\n");
        return;
      }

      stk::mesh::FieldBase* field = m_eMesh->getCoordinatesField();
      double *f_data = PerceptMesh::field_data(field, *node_ptr);

      f_data[0] = coordinates[0];
      f_data[1] = coordinates[1];
      f_data[2] = coordinates[2];

    }


    //============================================================================
    // Description: Each vertex has a byte-sized flag that can be used to store
    //! flags.  This byte's value is neither set nor used by the mesh
    //! implementation.  It is intended to be used by Mesquite algorithms.
    //! Until a vertex's byte has been explicitly set, its value is 0.
    //! Cubit stores the byte in a map associated with the node.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    void PerceptMesquiteMesh::vertex_set_byte (VertexHandle vertex,
                                               unsigned char byte,
                                               Mesquite::MsqError &err)
    {
      stk::mesh::Entity* node_ptr=reinterpret_cast<stk::mesh::Entity*>(vertex);
  
      //make sure there isn't a null pointer.
      if(node_ptr==NULL) {
        MSQ_SETERR(err)("PerceptMesquiteMesh::vertex_set_byte: invalid vertex handle.", Mesquite::MsqError::INVALID_STATE);
        PRINT_ERROR("PerceptMesquiteMesh::vertex_set_byte: invalid vertex handle.\n");
        return;
      }
  
      std::pair<stk::mesh::EntityId, unsigned char> msq_data = std::pair<stk::mesh::EntityId, unsigned char>(node_ptr->identifier(), byte);

      m_mesquiteNodeDataMap[node_ptr] = msq_data;
    }

    //============================================================================
    // Description: Set the byte for a given array of vertices.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    void PerceptMesquiteMesh::vertices_set_byte (
                                                 const VertexHandle *vert_array,
                                                 const unsigned char *byte_array,
                                                 size_t array_size,
                                                 Mesquite::MsqError &err)
    {
      //loop over the given vertices and call vertex_set_byte(...).
      size_t i=0;
      for(i=0;i<array_size;++i)
        {
          vertex_set_byte(vert_array[i],byte_array[i],err);
        }
    }

    //============================================================================
    // Description: retrieve the byte value for the specified vertex or vertices.
    //! The byte value is 0 if it has not yet been set via one of the
    //! *_set_byte() functions.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    void PerceptMesquiteMesh::vertex_get_byte(VertexHandle vertex,
                                              unsigned char *byte,
                                              Mesquite::MsqError &err)
    {
      stk::mesh::Entity* node_ptr=reinterpret_cast<stk::mesh::Entity*>(vertex);
  
      //make sure there isn't a null pointer.
      if(node_ptr==NULL) {
        MSQ_SETERR(err)("PerceptMesquiteMesh::vertex_get_byte: invalid vertex handle.", Mesquite::MsqError::INVALID_STATE);
        PRINT_ERROR("PerceptMesquiteMesh::vertex_get_byte: invalid vertex handle.");
        return;
      }
      *byte = m_mesquiteNodeDataMap[node_ptr].second;
    }

    //============================================================================
    // Description: get the bytes associated with the vertices in a given array.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================
    //! get the bytes associated with the vertices in a given array.
    void PerceptMesquiteMesh::vertices_get_byte(const VertexHandle *vertex_array,
                                                unsigned char *byte_array,
                                                size_t array_size,
                                                Mesquite::MsqError &err)
    {
      //loop over the given nodes and call vertex_get_byte(...)
      size_t i=0;
      for(i=0;i<array_size;++i){
        vertex_get_byte(vertex_array[i],&byte_array[i],err);
      }
  
    }

#if 0
    static bool hex_compare(CMLHex *h0, CMLHex *h1)
    {
      CubitVector a = h0->centroid();
      CubitVector b = h1->centroid();
      if (a.x() == b.x())
        {
          if (a.y() == b.y())
            {
              return a.z() < b.z();
            }
          return a.y() < b.y();
        }
      return a.x() < b.x();
    }

    static void sort_hexes(DLIList<CMLHex *> &initial, DLIList<CMLHex *> &final)
    {
      std::vector<CMLHex *> hex_vec;
      for (int ii=0; ii<initial.size(); ii++)
        {
          hex_vec.push_back(initial.get_and_step());
        }
      std::sort(hex_vec.begin(), hex_vec.end(), hex_compare);
      for( unsigned int jj=0; jj<hex_vec.size(); jj++)
        {
          final.append(hex_vec[jj]);
        }
    }
#endif

    //============================================================================
    // Description: Gets the elements attached to this vertex.
    //
    // elements: 	The array in which to place the handles of elements adjacent to 
    //            the input vertices.
    // offsets: 	For each vertex in vertex_array, the value in the corresponding 
    //            position in this array is the index into elem_array at which the 
    //            adjacency list begins for that vertex. 
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================  
    void PerceptMesquiteMesh::vertices_get_attached_elements(const VertexHandle* vertex_array,
                                                             size_t num_vertex,
                                                             std::vector<ElementHandle>& elements,
                                                             std::vector<size_t>& offsets,
                                                             Mesquite::MsqError& err )
    {
      size_t i=0;
      elements.clear();
      offsets.clear();
      stk::mesh::Entity* node_ptr=NULL;
      ElementHandle temp_e_handle;
      size_t offset_counter = 0;
  
      for(i=0; i<num_vertex; ++i)
        {
          offsets.push_back(offset_counter);
          node_ptr=reinterpret_cast<stk::mesh::Entity*>(vertex_array[i]);
    
          //make sure there isn't a null pointer
          if(node_ptr==NULL) {
            MSQ_SETERR(err)("PerceptMesquiteMesh::vertex_get_attached_elements: invalid vertex handle.", Mesquite::MsqError::INVALID_STATE);
            PRINT_ERROR("PerceptMesquiteMesh::vertex_get_attached_elements: invalid vertex handle.\n");
            return;
          }
    
          stk::mesh::PairIterRelation elements_of_node = node_ptr->relations(m_eMesh->element_rank());
          //sort_hexes( hex_patch, hex_list );  // to ensure parallel consistency

          for (unsigned iele=0; iele < elements_of_node.size(); iele++)
            {
              stk::mesh::Entity& ele = *elements_of_node[i].entity();

              // only return hexes in the includedElements set
              //if (includedElements.find(ent_ptr) != includedElements.end())
              {
                temp_e_handle = reinterpret_cast<ElementHandle>(&ele);
                elements.push_back(temp_e_handle);
                offset_counter++;
              }
            }
        }
      offsets.push_back(offset_counter);
  
    }


    //============================================================================
    // Description: Get the connectivity (ordered list of vertex handles) for each 
    //              element in the input array.
    //
    // elem_handles: The array of element handles for which to retrieve the connectivity list.
    // num_elems: The length of elem_handles
    // vert_handles: Array in which to place the vertex handles in each elements connectivity.
    // offsets: For each element in elem_handles, the value in the same position in 
    //          this array is the index into vert_handles at which the connectivity 
    //          list for that element begins. 
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================  
    void PerceptMesquiteMesh::elements_get_attached_vertices(const Mesquite::Mesh::ElementHandle *elem_handles,
                                                             size_t num_elems,
                                                             std::vector<Mesquite::Mesh::VertexHandle>& vert_handles,
                                                             std::vector<size_t> &offsets,
                                                             Mesquite::MsqError &err)
    {
      // Check for zero element case.
      vert_handles.clear();
      offsets.clear();
  
      if (num_elems == 0)
        {
          return;
        }      
  
      size_t i;
      stk::mesh::Entity* element_ptr;

  
      //get a list of all nodes that are in these elements (the elements
      // in the list will not necessarily be unique).
      size_t offset_counter = 0;
      for(i=0; i < ((size_t) num_elems);++i)
        {
          offsets.push_back(offset_counter);
          element_ptr = reinterpret_cast<stk::mesh::Entity*>(elem_handles[i]);

          VertexHandle temp_v_handle = NULL;

          stk::mesh::PairIterRelation nodes = element_ptr->relations(m_eMesh->node_rank());
          for (unsigned inode=0; inode < nodes.size(); inode++)
            {
              stk::mesh::Entity& node = *nodes[i].entity();
        
              temp_v_handle = reinterpret_cast<VertexHandle>(&node);
      
              if(temp_v_handle==NULL){
                PRINT_ERROR("Unexpected null pointer.\n");
                MSQ_SETERR(err)("Unexpected null pointer.",
                                Mesquite::MsqError::INVALID_STATE);
                return;
              }
              vert_handles.push_back(temp_v_handle);
              ++offset_counter;
            }
        }
      offsets.push_back(offset_counter);
  
    }

    //============================================================================
    // Description: Returns the topologies of the given entities. 
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================  
    void PerceptMesquiteMesh::elements_get_topologies(const ElementHandle *element_handle_array,
                                                      Mesquite::EntityTopology *element_topologies,
                                                      size_t num_elements,
                                                      Mesquite::MsqError &err)
    {
  
      stk::mesh::Entity *ent = NULL;
  
      //loop over the elements
      for ( ; num_elements--; )
        {
          ent = reinterpret_cast<stk::mesh::Entity*>(element_handle_array[num_elements]);
          if(ent==NULL){
            PRINT_ERROR("Unexpected null pointer.\n");
            MSQ_SETERR(err)("Unexpected null pointer.",
                            Mesquite::MsqError::INVALID_STATE);
            return;
          }
    
          const CellTopologyData * cell_topo_data = m_eMesh->get_cell_topology(*ent);
          if(cell_topo_data == NULL){
            PRINT_ERROR("Unexpected null topology.\n");
            MSQ_SETERR(err)("Unexpected null topology.",
                            Mesquite::MsqError::INVALID_STATE);
            return;
          }

          shards::CellTopology cell_topo(cell_topo_data);

          //add the appropriate EntityType to the element_topologies array
          if(cell_topo.getKey() == shards::getCellTopologyData<shards::Quadrilateral<4> >()->key )
            {
              element_topologies[num_elements]=Mesquite::QUADRILATERAL;
            }
          else if(cell_topo.getKey() == shards::getCellTopologyData<shards::Triangle<3> >()->key )
            {
              element_topologies[num_elements]=Mesquite::TRIANGLE;
            }
          else if(cell_topo.getKey() == shards::getCellTopologyData<shards::Hexahedron<8> >()->key )
            {
              element_topologies[num_elements]=Mesquite::HEXAHEDRON;
            }
          else if(cell_topo.getKey() == shards::getCellTopologyData<shards::Tetrahedron<4> >()->key )
            {
              element_topologies[num_elements]=Mesquite::TETRAHEDRON;
            }
          else {
            PRINT_ERROR("Type not recognized.\n");
            MSQ_SETERR(err)("Type not recognized.", Mesquite::MsqError::UNSUPPORTED_ELEMENT);
            return;
          }
    
        }//end loop over elements
  
    }

    //============================================================================
    // Description: Create a tag.
    // Create a user-defined data type that can be attached to any element or vertex 
    // in the mesh. For an opaque or undefined type, use type=BYTE and length=sizeof(..).
    // Note: Pure virtual inherits from Mesquite::Mesh
    // Author: sjowen, srkennon
    // Date: 04/14/2011
    //============================================================================
    Mesquite::TagHandle PerceptMesquiteMesh::tag_create(const std::string &tag_name,
                                                        Mesquite::Mesh::TagType type,
                                                        unsigned length,
                                                        const void* default_value,
                                                        Mesquite::MsqError &err)
    {
      Mesquite::TagHandle handle = 0;
#if 0
      if (tag_name == "msq_jacobi_temp_coords" && type == DOUBLE && length == 3)
        {
          if (jacobiCoords == NULL)
            {
              jacobiCoords = new double [3*numNodes];
              for (int ii=0; ii<numNodes*3; ii++)
                jacobiCoords[ii] = 0;
              nextJacobiCoordsIndex = 0;
              jacobiCoordsMap.clear();
            }
          handle = reinterpret_cast<Mesquite::TagHandle>(jacobiCoords);
        }
      else
        {
          PRINT_ERROR("Unknown Tag %s in tag_create\n", tag_name.c_str());
          MSQ_SETERR(err)("Tag not implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED);
        }
#endif
      return handle;
    }

    //============================================================================
    // Description: Remove a tag and all corresponding data. 
    // Delete a tag. 
    // Note: Pure virtual inherits from Mesquite::Mesh
    // Author: sjowen, srkennon
    // Date: 04/14/2011
    //============================================================================
    void PerceptMesquiteMesh::tag_delete(Mesquite::TagHandle handle,
                                         Mesquite::MsqError& err ) 
    {
#if 0
      Mesquite::TagHandle jhandle = reinterpret_cast<Mesquite::TagHandle>(jacobiCoords); 
      if (jhandle == handle)
        {
          delete [] jacobiCoords;
          jacobiCoords = NULL;
          nextJacobiCoordsIndex = 0;
          jacobiCoordsMap.clear();
        }
      else
        {
          PRINT_ERROR("Unknown tag sent to PerceptMesquiteMesh::tag_delete\n");
          MSQ_SETERR(err)("Unknown tag sent to PerceptMesquiteMesh::tag_delete\n",Mesquite::MsqError::NOT_IMPLEMENTED);
        }
#endif
    }

    //============================================================================
    // Description: 
    // Check for the existance of a tag given it's name and if it exists return a 
    // handle for it. If the specified tag does not exist, zero should be returned 
    // WITHOUT flagging an error.  
    // Note: Pure virtual inherits from Mesquite::Mesh
    // Author: sjowen, srkennon
    // Date: 04/14/2011
    //============================================================================
    Mesquite::TagHandle PerceptMesquiteMesh::tag_get(const std::string& name, 
                                                     Mesquite::MsqError& err )
    {
      Mesquite::TagHandle handle = 0;
#if 0
      if (name == "msq_jacobi_temp_coords")
        {
          if (jacobiCoords)
            handle = reinterpret_cast<Mesquite::TagHandle>(jacobiCoords);
        }
#endif
      return handle;
    }

    //============================================================================
    // Description: Get handle for existing tag, by name.
    // Get data type and number of values per entity for tag.  
    // Note: Pure virtual inherits from Mesquite::Mesh
    // Author: sjowen, srkennon
    // Date: 04/14/2011
    //============================================================================
    void PerceptMesquiteMesh::tag_properties(Mesquite::TagHandle handle,
                                             std::string &name_out,
                                             Mesquite::Mesh::TagType &type_out,
                                             unsigned &length_out,
                                             Mesquite::MsqError& err )
    {
#if 0
      Mesquite::TagHandle jhandle = reinterpret_cast<Mesquite::TagHandle>(jacobiCoords); 
      if (jhandle == handle)
        {
          name_out = "msq_jacobi_temp_coords";
          type_out = Mesquite::Mesh::DOUBLE;
          length_out = 3;
        }
      else
        { 
          MSQ_SETERR(err)("Unknown tag in PerceptMesquiteMesh::tag_properties.\n",Mesquite::MsqError::NOT_IMPLEMENTED);
        }
#endif
    }

    //============================================================================
    // Description: Set tag values on elements.
    // Set the value of a tag for a list of mesh elements.   
    // Note: Pure virtual inherits from Mesquite::Mesh
    // Author: sjowen, srkennon
    // Date: 04/14/2011
    //============================================================================
    void PerceptMesquiteMesh::tag_set_element_data(Mesquite::TagHandle /*handle*/,
                                                   size_t /*num_elems*/,
                                                   const Mesquite::Mesh::ElementHandle* /*elem_array*/,
                                                   const void* /*tag_data*/,
                                                   Mesquite::MsqError& err )
    {
      assert(0); 
      MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED);
    }

    //============================================================================
    // Description: Set tag values on vertices.
    // Set the value of a tag for a list of mesh vertices.    
    // Note: Pure virtual inherits from Mesquite::Mesh
    // Author: sjowen, srkennon
    // Date: 04/14/2011
    //============================================================================
    void PerceptMesquiteMesh::tag_set_vertex_data (Mesquite::TagHandle handle,
                                                   size_t num_nodes,
                                                   const Mesquite::Mesh::VertexHandle* node_array,
                                                   const void* tag_data,
                                                   Mesquite::MsqError& err )
    {
#if 0

      Mesquite::TagHandle jhandle = reinterpret_cast<Mesquite::TagHandle>(jacobiCoords); 
      if (jhandle != handle)
        {
          PRINT_ERROR("Unknown tag handle in PerceptMesquiteMesh::tag_set_vertex_data\n");
          MSQ_SETERR(err)("Unknown tag handle in PerceptMesquiteMesh::tag_set_vertex_data.\n",Mesquite::MsqError::NOT_IMPLEMENTED);
        }
      else
        {
          int inode;
          for(inode = 0; inode < (int)num_nodes; inode++)
            {
              double *coords = (double *)tag_data;
              int idx;
              Mesquite::Mesh::VertexHandle vhandle = node_array[inode];
              std::map<Mesquite::Mesh::VertexHandle, int>::iterator iter = jacobiCoordsMap.find(vhandle);
              if (iter == jacobiCoordsMap.end())
                {
                  assert(nextJacobiCoordsIndex < numNodes);
                  jacobiCoordsMap.insert(std::pair<Mesquite::Mesh::VertexHandle, int>(vhandle, nextJacobiCoordsIndex));
                  idx = nextJacobiCoordsIndex;
                  nextJacobiCoordsIndex++;
                }
              else
                {
                  idx = iter->second;
                }
              jacobiCoords[idx*3] = coords[0];
              jacobiCoords[idx*3+1] = coords[1];
              jacobiCoords[idx*3+2] = coords[2];
            }
        }
#endif
    }

    //============================================================================
    // Description: Get tag values on elements. 
    // Get the value of a tag for a list of mesh elements.     
    // Note: Pure virtual inherits from Mesquite::Mesh
    // Author: sjowen, srkennon
    // Date: 04/14/2011
    //============================================================================
    void PerceptMesquiteMesh::tag_get_element_data(Mesquite::TagHandle /*handle*/,
                                                   size_t /*num_elems*/,
                                                   const Mesquite::Mesh::ElementHandle* /*elem_array*/,
                                                   void* /*tag_data*/,
                                                   Mesquite::MsqError& err )
    {
      assert(0); 
      MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED);
    }

    //============================================================================
    // Description: Get tag values on vertices. 
    // Get the value of a tag for a list of mesh vertices.      
    // Note: Pure virtual inherits from Mesquite::Mesh
    // Author: sjowen, srkennon
    // Date: 04/14/2011
    //============================================================================
    void PerceptMesquiteMesh::tag_get_vertex_data (Mesquite::TagHandle handle,
                                                   size_t num_nodes,
                                                   const Mesquite::Mesh::VertexHandle* node_array,
                                                   void* tag_data,
                                                   Mesquite::MsqError& err )
    {
#if 0
      Mesquite::TagHandle jhandle = reinterpret_cast<Mesquite::TagHandle>(jacobiCoords); 
      if (jhandle != handle)
        {
          PRINT_ERROR("Unknown tag handle in PerceptMesquiteMesh::tag_get_vertex_data\n");
          MSQ_SETERR(err)("Unknown tag handle in PerceptMesquiteMesh::tag_get_vertex_data.\n",Mesquite::MsqError::NOT_IMPLEMENTED);
        }
      else
        {
          double *coords = (double *)tag_data;
          int inode;
          for(inode = 0; inode < (int)num_nodes; inode++)
            {
              int idx;
              Mesquite::Mesh::VertexHandle vhandle = node_array[inode];
              std::map<Mesquite::Mesh::VertexHandle, int>::iterator iter = jacobiCoordsMap.find(vhandle);
              if (iter == jacobiCoordsMap.end())
                {
                  MSQ_SETERR(err)("PerceptMesquiteMesh::tag_get_vertex_data: invalid vertex handle.",Mesquite::MsqError::INVALID_STATE);
                  PRINT_ERROR("PerceptMesquiteMesh::tag_get_vertex_data: invalid vertex handle.\n");
                  return;
                }
              else
                {
                  idx = iter->second;
                }
              coords[inode*3] = jacobiCoords[idx*3];
              coords[inode*3+1] = jacobiCoords[idx*3+1];
              coords[inode*3+2] = jacobiCoords[idx*3+2];
            }

        }
#endif
    }


    //============================================================================
    // Description: Tells the mesh that the client is finished with a given
    //              entity handle. 
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================  
    void PerceptMesquiteMesh::release_entity_handles(
                                                     const EntityHandle* /*handle_array*/,
                                                     size_t /*num_handles*/,
                                                     Mesquite::MsqError &/*err*/)
    {
      // Do nothing...
    }


    //============================================================================
    // Description: Instead of deleting a Mesh when you think you are done,
    //! call release().  In simple cases, the implementation could
    //! just call the destructor.  More sophisticated implementations
    //! may want to keep the Mesh object to live longer than Mesquite
    //! is using it.
    // Author: sjowen, srkennon
    // Date: 03/30/2011, 11/15/11
    //============================================================================  
    void PerceptMesquiteMesh::release()
    {
      // We allocate on the stack, so don't delete this...
      //  delete this;
    }

    //============================================================================
    // Description: return the location for a node stored in nodeCoords
    //              (used for storing updated nodal locations for Jacobi-type smoothing)
    // Author: sjowen, srkennon
    // Date: 4/4/2011
    //============================================================================  
#if 0
    bool PerceptMesquiteMesh::get_jacobi_vertex_coords(CMLNode *node_ptr, CubitVector &coords)
    {

      std::map<CMLNode *, CubitVector>::iterator iter = nodeCoords.find( node_ptr );
      if (iter == nodeCoords.end())
        {
          printf("#Jacobi Node Coords not set up for node %d.\n", node_ptr->id());
          return false;
        }
      coords = iter->second;
      return true;
    }
#endif



    //static bool myPerceptMesquiteMesh_cpp = true;


  } // namespace percept
} // namespace stk

#endif
#endif // STK_BUILT_IN_SIERRA
