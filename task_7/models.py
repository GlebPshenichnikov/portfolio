from abc import ABC, abstractmethod
from itertools import product
from typing import List, Tuple

import numpy as np

from preprocessing import TokenizedSentencePair


class BaseAligner(ABC):
    """
    Describes a public interface for word alignment models.
    """

    @abstractmethod
    def fit(self, parallel_corpus: List[TokenizedSentencePair]):
        """
        Estimate alignment model parameters from a collection of parallel sentences.

        Args:
            parallel_corpus: list of sentences with translations, given as numpy arrays of vocabulary indices

        Returns:
        """
        pass

    @abstractmethod
    def align(self, sentences: List[TokenizedSentencePair]) -> List[List[Tuple[int, int]]]:
        """
        Given a list of tokenized sentences, predict alignments of source and target words.

        Args:
            sentences: list of sentences with translations, given as numpy arrays of vocabulary indices

        Returns:
            alignments: list of alignments for each sentence pair, i.e. lists of tuples (source_pos, target_pos).
            Alignment positions in sentences start from 1.
        """
        pass


class DiceAligner(BaseAligner):
    def __init__(self, num_source_words: int, num_target_words: int, threshold=0.5):
        self.cooc = np.zeros((num_source_words, num_target_words), dtype=np.uint32)
        self.dice_scores = None
        self.threshold = threshold

    def fit(self, parallel_corpus):
        for sentence in parallel_corpus:
            # use np.unique, because for a pair of words we add 1 only once for each sentence
            for source_token in np.unique(sentence.source_tokens):
                for target_token in np.unique(sentence.target_tokens):
                    self.cooc[source_token, target_token] += 1
        self.dice_scores = (2 * self.cooc.astype(np.float32) /
                            (self.cooc.sum(0, keepdims=True) + self.cooc.sum(1, keepdims=True)))

    def align(self, sentences):
        result = []
        for sentence in sentences:
            alignment = []
            for (i, source_token), (j, target_token) in product(
                    enumerate(sentence.source_tokens, 1),
                    enumerate(sentence.target_tokens, 1)):
                if self.dice_scores[source_token, target_token] > self.threshold:
                    alignment.append((i, j))
            result.append(alignment)
        return result


class WordAligner(BaseAligner):
    def __init__(self, num_source_words, num_target_words, num_iters):
        self.num_source_words = num_source_words
        self.num_target_words = num_target_words
        self.translation_probs = np.full((num_source_words, num_target_words), 1 / num_target_words, dtype=np.float32)
        self.num_iters = num_iters

    def _e_step(self, parallel_corpus: List[TokenizedSentencePair]) -> List[np.array]:
        posteriors = []
        for pair in parallel_corpus:
            source = pair.source_tokens
            target = pair.target_tokens
            if len(source) == 0 or len(target) == 0:
                posteriors.append(np.zeros((0, 0)))
                continue

            probs = self.translation_probs[source[:, None], target]
            denominator = probs.sum(axis=0, keepdims=True)

            denominator[denominator == 0] = 1e-12
            posterior = probs / denominator

            posteriors.append(posterior.astype(np.float32))
        return posteriors

    def _compute_elbo(self, parallel_corpus: List[TokenizedSentencePair], posteriors: List[np.array]) -> float:
        elbo = 0
        for pair, posterior in zip(parallel_corpus, posteriors):
            n = len(pair.source_tokens)
            m = len(pair.target_tokens)
            if n == 0 or m == 0:
                continue

            elbo += m * np.log(1.0 / n)

            source = pair.source_tokens
            target = pair.target_tokens
            log_probs = np.log(self.translation_probs[source[:, None], target])
            log_probs = np.clip(log_probs, -1e12, 0)

            log_temp = np.log(posterior)
            log_temp = np.clip(log_temp, -1e12, 0)

            elbo += np.sum(posterior * log_probs) - np.sum(posterior * log_temp)

        return elbo

    def _m_step(self, parallel_corpus: List[TokenizedSentencePair], posteriors: List[np.array]):
        self.translation_probs.fill(0)
        for i, elem in enumerate(parallel_corpus):
            source = elem.source_tokens
            target = elem.target_tokens
            idx = np.ix_(source, target)
            np.add.at(self.translation_probs, idx, posteriors[i])
        self.translation_probs /= np.sum(self.translation_probs,
                                         axis=1)[:, None]
        return self._compute_elbo(parallel_corpus, posteriors)

    def fit(self, parallel_corpus):
        history = []
        for i in range(self.num_iters):
            posteriors = self._e_step(parallel_corpus)
            elbo = self._m_step(parallel_corpus, posteriors)
            history.append(elbo)
        return history

    def align(self, sentences):
        alignments = []

        for elem in sentences:
            posteriors = self._e_step([elem])[0]

            alignment = []
            s_len = len(elem.source_tokens)
            t_len = len(elem.target_tokens)

            for k in range(t_len):
                if s_len > 0:
                    j = np.argmax(posteriors[:, k])
                    alignment.append((j + 1, k + 1))

            alignments.append(alignment)

        return alignments


class WordPositionAligner(WordAligner):
    def __init__(self, num_source_words, num_target_words, num_iters):
        super().__init__(num_source_words, num_target_words, num_iters)
        self.alignment_probs = {}

    def _get_probs_for_lengths(self, src_length: int, tgt_length: int):
        """
        Given lengths of a source sentence and its translation, return the parameters of a "prior" distribution over
        alignment positions for these lengths. If these parameters are not initialized yet, first initialize
        them with a uniform distribution.

        Args:
            src_length: length of a source sentence
            tgt_length: length of a target sentence

        Returns:
            probs_for_lengths: np.array with shape (src_length, tgt_length)
        """
        pass

    def _e_step(self, parallel_corpus):
        pass

    def _compute_elbo(self, parallel_corpus, posteriors):
        pass

    def _m_step(self, parallel_corpus, posteriors):
        pass
