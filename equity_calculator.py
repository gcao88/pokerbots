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
    arr = [0] * 16  
    ranks = ['--23456789TJQKA'.index(r) for r, s in hand] 
    for r in ranks: 
        arr[r] += 1
    arr2 = [] 
    for x, y in enumerate(arr): 
        if y > 0:
            arr2.append((y, x)) 

    arr2.sort(key=lambda inp: (-inp[0], -inp[1]))
    ranks.sort(reverse=True)
    freqs = sorted([ranks.count(x) for x in set(ranks)], reverse=True)

    if ranks == [14, 5, 4, 3, 2]: # low straight
        ranks = [5, 4, 3, 2, 1]
    
    if len(set(r for r, s in hand)) == 5 and len(set(s for r, s in hand)) == 1 and max(ranks)-min(ranks) == 4: 
        return (8, arr2)  # straight flush
    elif freqs == [4, 1]: 
        return (7, arr2)  # four of a kind
    elif freqs == [3, 2]: 
        return (6, arr2)  # full house
    elif len(set(s for r, s in hand)) == 1: 
        return (5, arr2)  # flush
    elif len(set(ranks)) == 5 and max(ranks) - min(ranks) == 4: 
        return (4, arr2)  # straight
    elif freqs == [3, 1, 1]: 
        return (3, arr2)  # three of a kind
    elif freqs == [2, 2, 1]: 
        return (2, arr2)  # two pair
    elif freqs == [2, 1, 1, 1]: 
        return (1, arr2)  # one pair
    else: 
        return (0, arr2)  # high card

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

def calculate_conf(samples):
    confidence_level = 0.95
    degrees_freedom = len(samples) - 1
    sample_mean = np.mean(samples)
    sample_standard_error = stats.sem(samples)
    confidence_interval = stats.t.interval(confidence_level, degrees_freedom, sample_mean, sample_standard_error)
    return confidence_interval

def equity_vs_average_hand(h1, h2_size, board):
    deck = [r+s for r in '23456789TJQKA' for s in 'SHDC']
    for card in h1+board:
        deck.remove(card)

    equities = []
    for _ in range(30):
        h2 = random.sample(deck, h2_size)
        equities.append(get_equity(h1, h2, board))
    
    return sum(equities) / len(equities), calculate_conf(equities)

"""
h1 = ['AH', 'KS']
h2_size = 3
board = ['2C', '7D', 'KC']
print(equity_vs_average_hand(h1, h2_size, board))
"""

boards = [
    ['5D', '5C', '9D'],
    # ['KD', '9C', '2H'],
    # ['KD', '9C', '2C'],
    # ['6D', '7C', '8H'],
    # ['AD', 'KC', '4H'],
    # ['5D', '7C', 'TD'],
    # ['5D', '7C', 'TH'],
    # ['KD', '9D', '2D'],
    # ['KD', 'KC', '5C'],
]
# suit of cards: if suited, both cards are diamonds (or N/A if its already on board)
# otherwise, cards are spades/hearts or spades/clubs (i.e. no coordination with board)
ranks = 'AKQJT98765432'
for board in tqdm(boards):
    with open(f"equity_outputs/{''.join(board)}.csv", 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow([''] + list(ranks))
        for i, r1 in tqdm(enumerate(ranks)):
            equity_row = []
            for j, r2 in tqdm(enumerate(ranks)):
                if j > i:
                    if (r1 + 'D') in board or (r2 + 'D') in board:
                        equity_row.append('N/A')
                        continue
                    h1 = [r1 + 'D', r2 + 'D']
                else:
                    for s2 in 'HCS':
                        if (r1 + s2) not in board:
                            h1 = [r1 + s2, r2 + 'S']
                            break
                equity_row.append(equity_vs_average_hand(h1, 3, board)[0])
            writer.writerow([r1] + equity_row)