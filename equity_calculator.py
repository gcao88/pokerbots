import itertools
import numpy as np
from scipy import stats
import random
from tqdm import tqdm
import csv

def hand_strength(hand, board):
    all_cards = sorted(hand + board, reverse=True)
    return hand_value(max(itertools.combinations(all_cards, 5), key=hand_value))

def hand_value(hand):
    ranks = ['--23456789TJQKA'.index(r) for r, s in hand]
    ranks.sort(reverse=True)
    if len(set(r for r, s in hand)) == 5 and len(set(s for r, s in hand)) == 1 and max(ranks)-min(ranks) == 4: 
        return (8, ranks)  # straight flush
    elif len(set(ranks)) == 2: 
        return (7, ranks) if ranks.count(ranks[0]) in {1, 4} else (6, ranks)  # four of a kind or full house
    elif len(set(s for r, s in hand)) == 1: 
        return (5, ranks)  # flush
    elif len(set(ranks)) == 5 and max(ranks) - min(ranks) == 4: 
        return (4, ranks)  # straight
    elif len(set(ranks)) == 2 or len(set(ranks)) == 3: 
        return (3, ranks if ranks.count(ranks[2]) > 1 else ranks[::-1])  # three of a kind or two pair
    elif len(set(ranks)) == 4: 
        return (2, ranks if ranks.count(ranks[1]) > 1 else ranks[::-1])  # one pair
    else: 
        return (1, ranks)  # high card

def get_equity(h1, h2, board):
    deck = [r+s for r in '23456789TJQKA' for s in 'SHDC']
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

    return (wins + 0.5*ties) / cnt

def calculate_std_conf(samples):
    std = np.std(samples)
    confidence_level = 0.95
    degrees_freedom = len(samples) - 1
    sample_mean = np.mean(samples)
    sample_standard_error = stats.sem(samples)
    confidence_interval = stats.t.interval(confidence_level, degrees_freedom, sample_mean, sample_standard_error)
    return std, confidence_interval

def equity_vs_average_hand(h1, h2_size, board):
    deck = [r+s for r in '23456789TJQKA' for s in 'SHDC']
    for card in h1+board:
        deck.remove(card)

    equities = []
    for _ in range(100):
        h2 = random.sample(deck, h2_size)
        equities.append(get_equity(h1, h2, board))
    
    return sum(equities) / len(equities), calculate_std_conf(equities)

"""
h1 = ['KH', 'QH']
h2_size = 3
board = ['5C', 'KS', 'TC', '3C']
print(equity_vs_average_hand(h1, h2_size, board))
"""

# board cannot include spades, because h1 assumes spades are safe for hole cards
boards = [
    ['5D', '5C', '9D'],
    ['KD', '9C', '2H'],
    ['KD', '9C', '2C'],
    ['6D', '7C', '8H'],
    ['AD', 'KC', '4H'],
    ['5D', '7C', 'TD'],
    ['5D', '7C', 'TH'],
    ['KD', '9D', '2D'],
    ['KD', 'KC', '5C'],
]
ranks = 'AKQJT98765432'
for board in tqdm(boards):
    with open(f"equity_outputs/{''.join(board)}.csv", 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow([''] + list(ranks))
        for i, r1 in enumerate(ranks):
            equity_row = []
            for j, r2 in enumerate(ranks):
                if i > j:
                    h1 = [r1 + 'S', r2 + 'S']
                else:
                    for s in 'HCS':
                        if (r1 + s) not in board:
                            h1 = [r1 + s, r2 + 'S']
                            break
                equity_row.append(equity_vs_average_hand(h1, 3, board)[0])
            writer.writerow([r1] + equity_row)