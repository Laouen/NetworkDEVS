#if !defined abstract_types_h
#define abstract_types_h

namespace abstract {

  /**
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct Header abstract_types.h
   * 
   * @brief Represent an abstract header structure for any kind of network data.
   * @details This struct it is used to clasify the structs into header and data.
   * It does not implement any functionality.
   * 
   */
  struct Header {};

  /**
   * @author Laouen Louan Mayal Belloli
   * @date 14 May 2017
   * 
   * @struct Data abstract_types.h
   * 
   * @brief Represent an abstract data structure for any kind of network data.
   * @details This struct it is used to clasify the structs into header and data.
   * It does not implement any functionality.
   * 
   */
  struct Data {};
}

#endif