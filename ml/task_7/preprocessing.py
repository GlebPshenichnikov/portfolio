from dataclasses import dataclass
from typing import Dict, List, Tuple

import numpy as np
import xml.etree.ElementTree as ET


@dataclass(frozen=True)
class SentencePair:
    """
    Contains lists of tokens (strings) for source and target sentence
    """
    source: List[str]
    target: List[str]


@dataclass(frozen=True)
class TokenizedSentencePair:
    """
    Contains arrays of token vocabulary indices (preferably np.int32) for source and target sentence
    """
    source_tokens: np.ndarray
    target_tokens: np.ndarray


@dataclass(frozen=True)
class LabeledAlignment:
    """
    Contains arrays of alignments (lists of tuples (source_pos, target_pos)) for a given sentence.
    Positions are numbered from 1.
    """
    sure: List[Tuple[int, int]]
    possible: List[Tuple[int, int]]


def extract_sentences(filename: str) -> Tuple[List[SentencePair], List[LabeledAlignment]]:
    """
    Given a file with tokenized parallel sentences and alignments in XML format, return a list of sentence pairs
    and alignments for each sentence.

    Args:
        filename: Name of the file containing XML markup for labeled alignments

    Returns:
        sentence_pairs: list of `SentencePair`s for each sentence in the file
        alignments: list of `LabeledAlignment`s corresponding to these sentences
    """
    pairs = []
    alignments = []
    if filename is None:
        return [], []

    with open(filename, 'r') as file:
        f = file.read().replace('&', '&amp;')
    if f is None:
        return [], []

    root = ET.fromstring(f)
    if root is None:
        return [], []

    for elem in root:
        if elem is None:
            continue
        english = elem[0]
        czech = elem[1]
        if english is None or czech is None or english.text is None or czech.text is None:
            continue
        english = english.text.strip()
        czech = czech.text.strip()

        source_tokens = english.split()
        target_tokens = czech.split()
        pairs.append(SentencePair(source=source_tokens, target=target_tokens))

        sure = []
        possible = []

        s = elem[2]
        if s is not None and s.text is not None:
            sure = [tuple(map(int, pair.split('-'))) for pair in s.text.split()]

        p = elem[3]
        if p is not None and p.text is not None:
            possible = [tuple(map(int, pair.split('-'))) for pair in p.text.split()]

        alignments.append(LabeledAlignment(sure=sure, possible=possible))

    return pairs, alignments


def get_token_to_index(sentence_pairs: List[SentencePair], freq_cutoff=None) -> Tuple[Dict[str, int], Dict[str, int]]:
    """
    Given a parallel corpus, create two dictionaries token->index for source and target language.

    Args:
        sentence_pairs: list of `SentencePair`s for token frequency estimation
        freq_cutoff: if not None, keep only freq_cutoff -- natural number -- most frequent tokens in each language

    Returns:
        source_dict: mapping of token to a unique number (from 0 to vocabulary size) for source language
        target_dict: mapping of token to a unique number (from 0 to vocabulary size) target language

    Tip:
        Use cutting by freq_cutoff independently in src and target. Moreover in both cases of freq_cutoff (None or not None) - you may get a different size of the dictionary

    """
    source = {}
    target = {}

    for elem in sentence_pairs:
        for s in elem.source:
            source[s] = source.get(s, 0) + 1
        for t in elem.target:
            target[t] = target.get(t, 0) + 1

    source_sorted = sorted(source.items(), key=lambda x: x[1], reverse=True)
    target_sorted = sorted(target.items(), key=lambda x: x[1], reverse=True)

    if freq_cutoff is not None:
        source_sorted = source_sorted[:freq_cutoff]
        target_sorted = target_sorted[:freq_cutoff]

    ans_source = {}
    ans_target = {}
    for idx, (elem, cnt) in enumerate(source_sorted):
        ans_source[elem] = idx
    for idx, (elem, cnt) in enumerate(target_sorted):
        ans_target[elem] = idx

    return ans_source, ans_target


def tokenize_sents(sentence_pairs: List[SentencePair], source_dict, target_dict) -> List[TokenizedSentencePair]:
    """
    Given a parallel corpus and token_to_index for each language, transform each pair of sentences from lists
    of strings to arrays of integers. If either source or target sentence has no tokens that occur in corresponding
    token_to_index, do not include this pair in the result.

    Args:
        sentence_pairs: list of `SentencePair`s for transformation
        source_dict: mapping of token to a unique number for source language
        target_dict: mapping of token to a unique number for target language

    Returns:
        tokenized_sentence_pairs: sentences from sentence_pairs, tokenized using source_dict and target_dict
    """
    ans = []

    for elem in sentence_pairs:
        source = []
        target = []
        for s in elem.source:
            if s in source_dict:
                source.append(source_dict[s])

        for t in elem.target:
            if t in target_dict:
                target.append(target_dict[t])

        if source and target:
            ans.append(TokenizedSentencePair(
                source_tokens=np.array(source, dtype=np.int32),
                target_tokens=np.array(target, dtype=np.int32)
            ))

    return ans

