import itertools
import numpy as np
from scipy import stats
import random
from tqdm import tqdm
import csv
import eval7
import cython

def hand_strength(hand, board):
    all_cards = hand + board 
    eval7cards = []
    for card in all_cards: 
        eval7cards.append(eval7.Card(card))
    if len(eval7cards) == 7: 
        return eval7.evaluate(eval7cards)
    else: 
        return eval7.evaluate(eval7cards)
        foo = 0
        for i in range(8): 
            foo = max(foo, eval7.evaluate(eval7cards[0:i] + eval7cards[i:8]))
        return foo
# def hand_value(hand):
#     ranks = ['--23456789TJQKA'.index(r) for r, s in hand]
#     ranks.sort(reverse=True)
#     freqs = sorted([ranks.count(x) for x in set(ranks)], reverse=True)

#     if ranks == [14, 5, 4, 3, 2]: # low straight
#         ranks = [5, 4, 3, 2, 1]

#     if len(set(r for r, s in hand)) == 5 and len(set(s for r, s in hand)) == 1 and max(ranks)-min(ranks) == 4: 
#         return (8, ranks)  # straight flush
#     elif freqs == [4, 1]: 
#         return (7, ranks)  # four of a kind
#     elif freqs == [3, 2]: 
#         return (6, ranks)  # full house
#     elif len(set(s for r, s in hand)) == 1: 
#         return (5, ranks)  # flush
#     elif len(set(ranks)) == 5 and max(ranks) - min(ranks) == 4: 
#         return (4, ranks)  # straight
#     elif freqs == [3, 1, 1]: 
#         return (3, ranks)  # three of a kind
#     elif freqs == [2, 2, 1]: 
#         return (2, ranks)  # two pair
#     elif freqs == [2, 1, 1, 1]: 
#         return (1, ranks)  # one pair
#     else: 
#         return (0, ranks)  # high card

def get_equity(h1, h2, board):
    deck = [r+s for r in '23456789TJQKA' for s in 'shdc']
    for card in h1+h2+board:
        deck.remove(card)

    cnt, wins, losses, ties = 0, 0, 0, 0
    for b in itertools.combinations(deck, 5-len(board)):
        cnt += 1
        hand_1_strength = hand_strength(h1, list(b)+board)
        hand_2_strength = hand_strength(h2, list(b)+board)
        if hand_1_strength == hand_2_strength:
            ties += 1
        elif hand_1_strength > hand_2_strength:
            wins += 1
        else:
            losses += 1
    # if losses > wins: 
        # print(h2, board, (wins + 0.5*ties)/cnt)
    return (wins + 0.5*ties) / cnt

def calculate_conf(samples):
    confidence_level = 0.95
    degrees_freedom = len(samples) - 1
    sample_mean = np.mean(samples)
    sample_standard_error = stats.sem(samples)
    confidence_interval = stats.t.interval(confidence_level, degrees_freedom, sample_mean, sample_standard_error)
    return confidence_interval

def equity_vs_average_hand(h1, h2_size, board):
    deck = [r+s for r in '23456789TJQKA' for s in 'shdc']
    for card in h1+board:
        deck.remove(card)

    equities = []
    for _ in tqdm(range(80)):
        h2 = random.sample(deck, h2_size)
        equities.append(get_equity(h1, h2, board))
    
    return sum(equities) / len(equities), calculate_conf(equities)

# """
h1 = ['As', '9s']
h2_size = 3
board = ['5d', '6c', '9d']
print(equity_vs_average_hand(h1, h2_size, board))
# """

# board cannot include spades, because h1 assumes spades are safe for hole cards
# boards = [
#     ['5D', '5C', '9D'],
#     ['KD', '9C', '2H'],
#     ['KD', '9C', '2C'],
#     ['6D', '7C', '8H'],
#     ['AD', 'KC', '4H'],
#     ['5D', '7C', 'TD'],
#     ['5D', '7C', 'TH'],
#     ['KD', '9D', '2D'],
#     ['KD', 'KC', '5C'],
# ]
# ranks = 'AKQJT98765432'
# for board in tqdm(boards):
#     with open(f"equity_outputs/{''.join(board)}.csv", 'w', newline='') as file:
#         writer = csv.writer(file)
#         writer.writerow([''] + list(ranks))
#         for i, r1 in enumerate(ranks):
#             equity_row = []
#             for j, r2 in enumerate(ranks):
#                 if i > j:
#                     h1 = [r1 + 'S', r2 + 'S']
#                 else:
#                     for s in 'HCS':
#                         if (r1 + s) not in board:
#                             h1 = [r1 + s, r2 + 'S']
#                             break
#                 equity_row.append(equity_vs_average_hand(h1, 3, board)[0])
#             writer.writerow([r1] + equity_row)