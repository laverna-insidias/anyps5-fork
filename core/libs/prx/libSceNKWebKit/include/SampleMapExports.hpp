#pragma once

#include "SceTypes.hpp"
#include "prx/libSceNKWebKit/src/SampleMapAbi.hpp"

extern "C" {

bool APS5_VABI _ZNK7WebCore9SampleMap5emptyEv(const NKWebKit::SampleMapAbi::SampleMap* self);
void APS5_VABI _ZN7WebCore9SampleMap5clearEv(NKWebKit::SampleMapAbi::SampleMap* self);
void APS5_VABI _ZN7WebCore9SampleMap9addSampleERNS_11MediaSampleE(NKWebKit::SampleMapAbi::SampleMap* self, NKWebKit::SampleMapAbi::MediaSample& sample);
void APS5_VABI _ZN7WebCore9SampleMap12removeSampleEPNS_11MediaSampleE(NKWebKit::SampleMapAbi::SampleMap* self, NKWebKit::SampleMapAbi::MediaSample* sample);
NKWebKit::SampleMapAbi::TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap30findSampleWithPresentationTimeERKN3WTF9MediaTimeE(NKWebKit::SampleMapAbi::PresentationOrderSampleMap* self, const WTF::MediaTime& time);
NKWebKit::SampleMapAbi::TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap36findSampleContainingPresentationTimeERKN3WTF9MediaTimeE(NKWebKit::SampleMapAbi::PresentationOrderSampleMap* self, const WTF::MediaTime& time);
NKWebKit::SampleMapAbi::TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap43findSampleContainingOrAfterPresentationTimeERKN3WTF9MediaTimeE(NKWebKit::SampleMapAbi::PresentationOrderSampleMap* self, const WTF::MediaTime& time);
NKWebKit::SampleMapAbi::TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap43findSampleStartingOnOrAfterPresentationTimeERKN3WTF9MediaTimeE(NKWebKit::SampleMapAbi::PresentationOrderSampleMap* self, const WTF::MediaTime& time);
NKWebKit::SampleMapAbi::TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap39findSampleStartingAfterPresentationTimeERKN3WTF9MediaTimeE(NKWebKit::SampleMapAbi::PresentationOrderSampleMap* self, const WTF::MediaTime& time);
NKWebKit::SampleMapAbi::ReverseIterator APS5_VABI _ZN7WebCore26PresentationOrderSampleMap43reverseFindSampleContainingPresentationTimeERKN3WTF9MediaTimeE(NKWebKit::SampleMapAbi::PresentationOrderSampleMap* self, const WTF::MediaTime& time);
NKWebKit::SampleMapAbi::ReverseIterator APS5_VABI _ZN7WebCore26PresentationOrderSampleMap39reverseFindSampleBeforePresentationTimeERKN3WTF9MediaTimeE(NKWebKit::SampleMapAbi::PresentationOrderSampleMap* self, const WTF::MediaTime& time);
NKWebKit::SampleMapAbi::IteratorRange APS5_VABI _ZN7WebCore26PresentationOrderSampleMap35findSamplesBetweenPresentationTimesERKN3WTF9MediaTimeES4_(NKWebKit::SampleMapAbi::PresentationOrderSampleMap* self, const WTF::MediaTime& beginTime, const WTF::MediaTime& endTime);
NKWebKit::SampleMapAbi::IteratorRange APS5_VABI _ZN7WebCore26PresentationOrderSampleMap42findSamplesBetweenPresentationTimesFromEndERKN3WTF9MediaTimeES4_(NKWebKit::SampleMapAbi::PresentationOrderSampleMap* self, const WTF::MediaTime& beginTime, const WTF::MediaTime& endTime);
NKWebKit::SampleMapAbi::TreeNodeBase* APS5_VABI _ZN7WebCore20DecodeOrderSampleMap23findSampleWithDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(NKWebKit::SampleMapAbi::DecodeOrderSampleMap* self, const NKWebKit::SampleMapAbi::DecodeKey& key);
NKWebKit::SampleMapAbi::TreeNodeBase* APS5_VABI _ZN7WebCore20DecodeOrderSampleMap24findSampleAfterDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(NKWebKit::SampleMapAbi::DecodeOrderSampleMap* self, const NKWebKit::SampleMapAbi::DecodeKey& key);
NKWebKit::SampleMapAbi::ReverseIterator APS5_VABI _ZN7WebCore20DecodeOrderSampleMap30reverseFindSampleWithDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(NKWebKit::SampleMapAbi::DecodeOrderSampleMap* self, const NKWebKit::SampleMapAbi::DecodeKey& key);
NKWebKit::SampleMapAbi::ReverseIterator APS5_VABI _ZN7WebCore20DecodeOrderSampleMap37findSyncSamplePriorToPresentationTimeERKN3WTF9MediaTimeES4_(NKWebKit::SampleMapAbi::DecodeOrderSampleMap* self, const WTF::MediaTime& time, const WTF::MediaTime& threshold);
NKWebKit::SampleMapAbi::TreeNodeBase* APS5_VABI _ZN7WebCore20DecodeOrderSampleMap35findSyncSampleAfterPresentationTimeERKN3WTF9MediaTimeES4_(NKWebKit::SampleMapAbi::DecodeOrderSampleMap* self, const WTF::MediaTime& time, const WTF::MediaTime& threshold);
NKWebKit::SampleMapAbi::ReverseIteratorRange APS5_VABI _ZN7WebCore20DecodeOrderSampleMap20findDependentSamplesEPNS_11MediaSampleE(NKWebKit::SampleMapAbi::DecodeOrderSampleMap* self, NKWebKit::SampleMapAbi::MediaSample* sample);
NKWebKit::SampleMapAbi::IteratorRange APS5_VABI _ZN7WebCore20DecodeOrderSampleMap28findSamplesBetweenDecodeKeysERKSt4pairIN3WTF9MediaTimeES3_ES6_(NKWebKit::SampleMapAbi::DecodeOrderSampleMap* self, const NKWebKit::SampleMapAbi::DecodeKey& beginKey, const NKWebKit::SampleMapAbi::DecodeKey& endKey);

}
