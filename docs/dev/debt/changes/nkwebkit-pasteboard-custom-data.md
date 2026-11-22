# libSceNKWebKit

## add Functional

- [WebCore::PasteboardCustomData and its Entry](../../../core/libs/prx/libSceNKWebKit/src/PasteboardCustomData.cpp) (libSceNKWebKit) - not compared with a console module. The constructors, assignments and destructor follow the pinned [PasteboardCustomData.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/platform/PasteboardCustomData.cpp): an origin string and a vector of entries, each a type, custom data and a `String` or `Ref<SharedBuffer>` variant with a one-byte index. Copies share strings and buffers by reference count, and the last reference to a buffer goes through the `SharedBuffer` destructor export. The layouts are unverified. `createSharedBuffer`, `fromSharedBuffer` and `fromPersistenceDecoder` still throw. Closes when: compared with a console module.

## remove | `_ZN7WebCore20PasteboardCustomData5EntryC1EOS1_` | `rLV7ZbjxqD0` |

## remove | `_ZN7WebCore20PasteboardCustomData5EntryC1ERKS1_` | `jqLCVPYZF24` |

## remove | `_ZN7WebCore20PasteboardCustomData5EntryC1Ev` | `8q0o+-HUPBE` |

## remove | `_ZN7WebCore20PasteboardCustomData5EntryC2EOS1_` | `TaCxSAxB9U8` |

## remove | `_ZN7WebCore20PasteboardCustomData5EntryC2ERKS1_` | `3Ts9sXMsLDg` |

## remove | `_ZN7WebCore20PasteboardCustomData5EntryC2Ev` | `GAOGiBWWxD0` |

## remove | `_ZN7WebCore20PasteboardCustomData5EntryaSEOS1_` | `jVdD5Xr7il4` |

## remove | `_ZN7WebCore20PasteboardCustomData5EntryaSERKS1_` | `Xksn+zj4x60` |

## remove | `_ZN7WebCore20PasteboardCustomDataC1EON3WTF6StringEONS1_6VectorINS0_5EntryELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEE` | `ec46hZwJups` |

## remove | `_ZN7WebCore20PasteboardCustomDataC1EOS0_` | `71MqbV1TZQA` |

## remove | `_ZN7WebCore20PasteboardCustomDataC1ERKS0_` | `HvoRj7fP+dU` |

## remove | `_ZN7WebCore20PasteboardCustomDataC1Ev` | `cSUfCJp7YEw` |

## remove | `_ZN7WebCore20PasteboardCustomDataC2EON3WTF6StringEONS1_6VectorINS0_5EntryELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEE` | `y-m7ObwVY-8` |

## remove | `_ZN7WebCore20PasteboardCustomDataC2EOS0_` | `HuHO7Bn9W5k` |

## remove | `_ZN7WebCore20PasteboardCustomDataC2ERKS0_` | `ymWO6sZglMw` |

## remove | `_ZN7WebCore20PasteboardCustomDataC2Ev` | `8htex21zoHw` |

## remove | `_ZN7WebCore20PasteboardCustomDataD1Ev` | `jgrJPZW1x8o` |

## remove | `_ZN7WebCore20PasteboardCustomDataD2Ev` | `KOgTsORWgd4` |

## remove | `_ZN7WebCore20PasteboardCustomDataaSERKS0_` | `CbNwLqSqKDw` |
