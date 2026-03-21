### Alembic (ILM open-source geometry caching library, downloaded via FetchContent)
set (EZ_3RDPARTY_ALEMBIC_SUPPORT ON CACHE BOOL "Whether to add support for Alembic (.abc) file import. Downloads Alembic automatically via FetchContent.")
mark_as_advanced(FORCE EZ_3RDPARTY_ALEMBIC_SUPPORT)

macro(ez_requires_alembic)
  ez_requires(EZ_3RDPARTY_ALEMBIC_SUPPORT)
  ez_requires(EZ_3RDPARTY_IMATH_SUPPORT)
endmacro()
