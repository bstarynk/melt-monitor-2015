// file nitem.cc - managing items, new way

/**   Copyright (C)  2016  Basile Starynkevitch and later the FSF
      MONIMELT is a monitor for MELT - see http://gcc-melt.org/
      This file is part of GCC.

      GCC is free software; you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation; either version 3, or (at your option)
      any later version.

      GCC is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.
      You should have received a copy of the GNU General Public License
      along with GCC; see the file COPYING3.   If not see
      <http://www.gnu.org/licenses/>.
**/

#include "meltmoni.h"
#include <unordered_map>
#include <thread>
#include <mutex>


struct Mom_itemaddr_t
{
  // we encode an address by negating its bits to fool the conservative GC
  intptr_t itad_ptrbits;
  Mom_itemaddr_t(void*p=nullptr) : itad_ptrbits(~(intptr_t)p) {};
  ~Mom_itemaddr_t()
  {
    itad_ptrbits=~(intptr_t)MOM_EMPTY_SLOT;
  };
  Mom_itemaddr_t(const Mom_itemaddr_t&) = default;
  Mom_itemaddr_t(Mom_itemaddr_t&&) = default;
  void* ptr() const { return (void*)(~itad_ptrbits); };
};

GC_DECLARE_PTRFREE(Mom_itemaddr_t);

static inline momhash_t
hash_item_from_hid_and_loid_MOM(uint16_t hid, uint64_t loid)
{
  if (MOM_UNLIKELY(hid==0 || loid==0))
    MOM_FATAPRINTF("hash_item_from_hid_and_lid: invalid hid=%u loid=%llu",
                   (unsigned) hid, (unsigned long long) loid);
  momhash_t h =
    41 * hid +
    ((643 * (uint32_t) (loid >> 32)) ^
     (839 * (uint32_t) (loid & 0xffffffffULL) - hid % 9839));
  if (MOM_UNLIKELY(h == 0))
    {
      h = (loid % 1000000663ULL) + (hid % 45179U) + 4;
      assert (h>0);
    }
  return h;
}

struct mom_idpair_t
{
  uint16_t id_hid;
  uint64_t id_loid;
};

struct Mom_idpairhash_oper
{
  momhash_t operator() (const struct mom_idpair_t& p) const
  {
    return hash_item_from_hid_and_loid_MOM(p.id_hid,p.id_loid);
  };
};

struct Mom_idpairequal_oper
{
  constexpr bool operator() (const struct mom_idpair_t& l,
                             const struct mom_idpair_t& r) const
  {
    return l.id_hid == r.id_hid && l.id_loid == r.id_loid;
  }
};

typedef std::unordered_map<mom_idpair_t, Mom_itemaddr_t,
			   Mom_idpairhash_oper,
			   Mom_idpairequal_oper,
			   gc_allocator<Mom_itemaddr_t>
			   > mom_itembucket_t;

static constexpr const unsigned Mom_nbitem_buckets = 401; // a prime

static std::array<mom_itembucket_t,Mom_nbitem_buckets> mom_item_bucket_arr;

static std::array<std::mutex,Mom_nbitem_buckets> mom_item_mtx_arr;

constexpr static inline unsigned
mom_item_indexbu_from_id (uint16_t hid, uint64_t loid)
{
  return (loid + (unsigned)hid) % Mom_nbitem_buckets;
}

static void cleanup_item_MOM(void *itmad, void *clad)
{
  assert (itmad != NULL);
  assert (clad == NULL);
  auto itmdata = (momty_item_ty*)itmad;
  #warning cleanup_item_MOM unimplemented
  MOM_FATAPRINTF("cleanup_item_MOM unimplemented itmad=%p", itmad);
  pthread_mutex_destroy (&itmdata->momfi_itm_mtx);
} // end cleanup_item_MOM

/// momf_make_item is declared in header_module
extern "C" const char momsig_make_item[] = "signature_space_to_item";
extern "C" momitemptr_t momf_make_item(momty_space_en spa)
{
  uint16_t hid=0;
  uint64_t loid=0;
  momitemptr_t itm = nullptr;
  do
    {
      do
        {
          hid = mom_random_uint32 () & 0xffff;
          if (MOM_UNLIKELY(hid<10))
            continue;
          loid =
            (((uint64_t) (mom_random_uint32 ())) << 32) |
            (mom_random_uint32 ());
        }
      while (loid<10);
      unsigned bix = mom_item_indexbu_from_id(hid, loid);
      std::lock_guard<std::mutex> gu(mom_item_mtx_arr[bix]);
      auto& buck= mom_item_bucket_arr[bix];
      auto it = buck.find(mom_idpair_t {hid,loid});
      if (MOM_LIKELY(it == buck.end()))
        {
	  auto itmdata = (momty_item_ty*)mom_gc_alloc(sizeof(momty_item_ty));
	  itmdata->momfi_va_itype = momenuva_ity_item;
	  itmdata->momfi_va_ixv =  spa;
	  itmdata->momfi_hva_hash = hash_item_from_hid_and_loid_MOM(hid,loid);
	  itmdata->momfi_itm_hid = hid;
	  itmdata->momfi_itm_lid = loid;
	  pthread_mutex_init (&itmdata->momfi_itm_mtx, &mom_item_mtxattr);
	  time(&itmdata->momfi_itm_mtime);
	  itm = (momitemptr_t) itmdata;
          buck.emplace(mom_idpair_t {hid,loid},Mom_itemaddr_t {itm});
	  GC_REGISTER_FINALIZER_IGNORE_SELF (itmdata, cleanup_item_MOM, NULL, NULL,
				       NULL);
        }
    }
  while (itm == nullptr);

#warning momf_make_item unimplemented
} // end momf_make_item

// momf_make_item_from_id is declared in header_module
extern "C" const char momsig_make_item_from_id[] = "signature_hid_loid_space_to_item";
extern "C" momitemptr_t momf_make_item_from_id(uint16_t lid, uint64_t hid, momty_space_en spa)
{
  if (MOM_UNLIKELY(lid == 0 || hid == 0))
    {
      char buf[MOM_HI_LO_SUFFIX_LEN];
      memset (buf, 0, sizeof(buf));
      MOM_WARNPRINTF("make_item_from_id: invalid lid=%u hid=%llu [%s]",
                     (unsigned)lid, (unsigned long long)hid,
                     mom_hi_lo_suffix(buf, lid, hid));
      return nullptr;
    }
#warning momf_make_item_from_id unimplemented
} // end momf_make_item_from_id

// momf_find_item_of_id is declared in header_module
extern "C" const char momsig_find_item_of_id[] = "signature_hid_loid_to_item";
momitemptr_t momf_find_item_of_id(uint16_t lid, uint64_t hid, momty_space_en spa)
{
  if (MOM_UNLIKELY(lid == 0 || hid == 0))
    {
      char buf[MOM_HI_LO_SUFFIX_LEN];
      memset (buf, 0, sizeof(buf));
      MOM_WARNPRINTF("find_item_of_id: invalid lid=%u hid=%llu [%s]",
                     (unsigned)lid, (unsigned long long)hid,
                     mom_hi_lo_suffix(buf, lid, hid));
      return nullptr;
    }
#warning momf_find_item_of_id unimplemented
} // end momf_find_item_of_id
