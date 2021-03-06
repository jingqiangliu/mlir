//===- AttributeSupport.h ---------------------------------------*- C++ -*-===//
//
// Copyright 2019 The MLIR Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================
//
// This file defines support types for registering dialect extended attributes.
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_IR_ATTRIBUTESUPPORT_H
#define MLIR_IR_ATTRIBUTESUPPORT_H

#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/StorageUniquerSupport.h"
#include "llvm/ADT/PointerIntPair.h"

namespace mlir {
class MLIRContext;
class Type;

//===----------------------------------------------------------------------===//
// AttributeStorage
//===----------------------------------------------------------------------===//

namespace detail {
class AttributeUniquer;
} // end namespace detail

/// Base storage class appearing in an attribute. Derived storage classes should
/// only be constructed within the context of the AttributeUniquer.
class AttributeStorage : public StorageUniquer::BaseStorage {
  friend detail::AttributeUniquer;
  friend StorageUniquer;

public:
  /// Returns if the derived attribute is or contains a function pointer.
  bool isOrContainsFunctionCache() const {
    return typeAndContainsFunctionAttrPair.getInt();
  }

  /// Get the type of this attribute.
  Type getType() const;

  /// Get the dialect of this attribute.
  const Dialect &getDialect() const {
    assert(dialect && "Malformed attribute storage object.");
    return *dialect;
  }

protected:
  /// Construct a new attribute storage instance with the given type and a
  /// boolean that signals if the derived attribute is or contains a function
  /// pointer.
  /// Note: All attributes require a valid type. If no type is provided here,
  ///       the type of the attribute will automatically default to NoneType
  ///       upon initialization in the uniquer.
  AttributeStorage(Type type, bool isOrContainsFunctionCache = false);
  AttributeStorage(bool isOrContainsFunctionCache);
  AttributeStorage();

  /// Set the type of this attribute.
  void setType(Type type);

  // Set the dialect for this storage instance. This is used by the
  // AttributeUniquer when initializing a newly constructed storage object.
  void initializeDialect(const Dialect &newDialect) { dialect = &newDialect; }

private:
  /// The dialect for this attribute.
  const Dialect *dialect;

  /// This field is a pair of:
  ///  - The type of the attribute value.
  ///  - A boolean that is true if this is, or contains, a function attribute.
  llvm::PointerIntPair<const void *, 1, bool> typeAndContainsFunctionAttrPair;
};

/// Default storage type for attributes that require no additional
/// initialization or storage.
using DefaultAttributeStorage = AttributeStorage;

//===----------------------------------------------------------------------===//
// AttributeStorageAllocator
//===----------------------------------------------------------------------===//

// This is a utility allocator used to allocate memory for instances of derived
// Attributes.
using AttributeStorageAllocator = StorageUniquer::StorageAllocator;

//===----------------------------------------------------------------------===//
// AttributeUniquer
//===----------------------------------------------------------------------===//
namespace detail {
// A utility class to get, or create, unique instances of attributes within an
// MLIRContext. This class manages all creation and uniquing of attributes.
class AttributeUniquer {
public:
  /// Get an uniqued instance of attribute T.
  template <typename T, typename... Args>
  static T get(MLIRContext *ctx, unsigned kind, Args &&... args) {
    return ctx->getAttributeUniquer().get<typename T::ImplType>(
        getInitFn(ctx, T::getClassID()), kind, std::forward<Args>(args)...);
  }

  /// Erase a uniqued instance of attribute T.
  template <typename T, typename... Args>
  static void erase(MLIRContext *ctx, unsigned kind, Args &&... args) {
    return ctx->getAttributeUniquer().erase<typename T::ImplType>(
        kind, std::forward<Args>(args)...);
  }

private:
  /// Returns a functor used to initialize new attribute storage instances.
  static std::function<void(AttributeStorage *)>
  getInitFn(MLIRContext *ctx, const ClassID *const attrID);
};
} // namespace detail

} // end namespace mlir

#endif
