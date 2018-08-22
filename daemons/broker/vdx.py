import sys
import logging
import msgpack
import gurobipy
import numpy as np

from multiprocessing import Pool, cpu_count
from collections import defaultdict
from copy import copy
from scipy.spatial import distance

import scenario

METHOD = 'Exchange'

METHODS = ['Brokered', '2Clusters', '100Clusters',
           'DynamicPricing', 'DynamicMulticluster',
           'BestPhonebook', 'Exchange', 'Optimal']

METHOD_SETTINGS = {
    # num_bids, use_cost, use_capacity, expose_clients
    'Brokered': [1, False, False, False],
    '2Clusters': [2, False, False, False],
    '100Clusters': [100, False, False, False],
    'DynamicPricing': [1, True, False, False],
    'DynamicMulticluster': [100, True, False, False],
    'BestPhonebook': [100, True, True, False],
    'Exchange': [100, True, True, True],
    'Optimal': [-1, True, True, True],
}

EXPOSE_COST = 'FULL'

BG_TRAFFIC_PERCENTAGE = 4.0

# weights for optimization
w_cost = 0.01           # CDN costs modifier
w_perf = 1.0 - w_cost  # overall performance modifier
w_bw   = .01            # bandwidth modifier
w_ping = -10          # ping latency/drop modifier

closest_clusters_lookup = {}
bid_ordering = defaultdict(dict)


def get_regression_score(location, cluster):
    l = Scenario['client_locations'][location]
    c = Scenario['cdn_locations'][cluster]
    d = distance.cdist([(l['lat'], l['lon'])], [(c['lat'], c['lon'])])[0][0]
    m, b = l['regression']
    return max(0.01, m * d + b)


def build_bid_ordering(cl):
    cdn, location = cl
    cluster_scores = Scenario['client_locations'][location]['cluster_scores']

    bo = [(c, max(0.01, s), r) for c, s, r in cluster_scores
          if c in Scenario['CDNs'][cdn]]
    unseen_clusters = list(Scenario['CDNs'][cdn] -
                           set(zip(*cluster_scores)[0]))
    bo += [(cluster, get_regression_score(location, cluster))
           for cluster in unseen_clusters]
    bo = sorted(bo, key=lambda x: x[1])
    return (cdn, location, bo)


def get_closest_clusters(cluster):
    c = Scenario['cdn_locations'][cluster]
    ll = (c['lat'], c['lon'])
    if ll in closest_clusters_lookup:
        return (cluster, closest_clusters_lookup[ll])
    cc = []
    for other_cluster, c2 in Scenario['cdn_locations'].items():
        if cluster == other_cluster:
            continue
        dist = distance.cdist([ll], [(c2['lat'], c2['lon'])])[0][0]
        if dist < 3.0:  # approx 200 miles
            cc.append((cluster, max(dist, 0.1)))
    cc = sorted(cc, key=lambda x: x[1])
    closest_clusters_lookup[ll] = cc
    return (cluster, cc)


def setup(scenario):
    global Scenario, bid_ordering
    global w_ping, w_bw

    Scenario = scenario.scenario

    # set the criteria for how we calculate bids (default = use both)
    print '\n\n\n', scenario.mode()
    if scenario.mode() == 'ping':
        # ping/latency only
        w_bw = 0
    elif scenario.mode() == 'rate':
        # bandwidth only
        w_ping = 0

    print 'values', w_ping, w_bw

    print scenario.mode()

    for cdn in xrange(len(Scenario['CDNs'])):
        Scenario['CDNs'][cdn] = set(Scenario['CDNs'][cdn])

    p = Pool(cpu_count())

    logging.debug('calculating bid_ordering')
    bo = p.map(build_bid_ordering, [
        (cdn, location) for location in Scenario['client_locations'] for
        cdn in xrange(len(Scenario['CDNs']))])
    bid_ordering = defaultdict(dict)
    for cdn, location, ordering in bo:
        bid_ordering[cdn][location] = ordering

    logging.debug('calculating cluster distances')
    closest_clusters = dict(p.map(get_closest_clusters,
                                  Scenario['cdn_locations']))

    p.close()

def GetCDNBids(cdn):
    if len(Scenario['CDNs'][cdn]) == 0:
        return {}

    logging.debug('generating bids for CDN %s' % cdn)

    bids = defaultdict(list)

    choices = METHOD_SETTINGS[METHOD][0]
    if choices == -1:
        choices = len(Scenario['cdn_locations'])

    for location in Scenario['client_locations']:
        # adjust bid ordering to pick locations
        # with scores below 'cap' (function of best score)
        # sorted by lowest cost


        # modified this code so that it works in a live scenario, not just a static one
	cluster_scores = Scenario['client_locations'][location]['cluster_scores']

        # ignore links that are down (score == 0)
	bo = [(c, s, r) for c,s,r in cluster_scores if c in Scenario['CDNs'][cdn] and s != 0]
        if len(bo) == 0:
            return {}
	bo = sorted(bo, key = lambda x: x[1])

        cap = bo[0][1] * 2.0
        b2 = sorted([(c, s, r, Scenario['cdn_locations'][c]['bw_cost'] +
                      Scenario['cdn_locations'][c]['colo_cost'])
                     for c, s, r in bo if s <= cap], key=lambda x: x[2])

        if len(b2) > 1:
            bo = b2
        else:  # if there's only one choice at this point...
            # just give me the best two scoring choices, sorted by cost
            bo = sorted([(c, s, r, Scenario['cdn_locations'][c]['bw_cost'] +
                          Scenario['cdn_locations'][c]['colo_cost'])
                         for c, s, r in bo[:3]], key=lambda x: x[2])
        bo = [(c, s, r) for (c, s, r, _) in bo]

        if METHOD == "Optimal":
            bo = bid_ordering[cdn][location]

        selected = 0
        for (cluster, score, rate) in bo:
            if score == 0:
                capacity = 0
            else:
                capacity = Scenario['capacities'][(cdn, cluster)]
            if not METHOD_SETTINGS[METHOD][3]:
                capacity *= BG_TRAFFIC_PERCENTAGE
            else:
                capacity *= 0.7  # protect from overages in optimization
            c = Scenario['cdn_locations'][cluster]
            bids[location].append((cdn, cluster, score, capacity,
                                   c['bw_cost'], c['colo_cost'], rate))
            selected += 1
            if selected >= choices:
                break

    bids = dict(bids)

    # TODO: better alternate bidding strats
    bs = []
    for location in bids:
        for i in xrange(len(bids[location])):
            _, _, _, _, bw_cst, colo_cst, _ = bids[location][i]
            total = bw_cst + colo_cst
            bs.append((total, bw_cst, colo_cst, location, bids[location][i]))

    bids = defaultdict(list)
    bs = sorted(bs, key=lambda x: x[0])

    avbw = float(sum(zip(*bs)[0])) / len(bs)
    avcolo = float(sum(zip(*bs)[0])) / len(bs)
    # should be...
    # avbw = float(sum(zip(*bs)[1])) / len(bs)
    # avcolo = float(sum(zip(*bs)[2])) / len(bs)

    a = range(len(bs))
    a = (1.0 * sum(a)) / len(a)
    for i, (_, _, _, location, bid) in enumerate(bs):
        cdn, cluster, score, capacity, bw_cst, colo_cst, rate = bid
        standard_price = Scenario['CDN_standard_price'][
            cdn] if cdn in Scenario['CDN_standard_price'] else 1
        if EXPOSE_COST == 'FULL':
            bids[location].append((cdn, cluster, score, capacity,
                                   bw_cst, colo_cst, rate))
        if EXPOSE_COST == 'NOTHING':
            bids[location].append((cdn, cluster, score, capacity,
                                   standard_price, standard_price, rate))
        if EXPOSE_COST == 'OPAQUE':
            bids[location].append((cdn, cluster, score, capacity,
                                   0.5 * i * standard_price / a,
                                   0.5 * i * standard_price / a, rate))
        if EXPOSE_COST == 'RELATIVE':
            bids[location].append((cdn, cluster, score, capacity,
                                   bw_cst / avbw * standard_price,
                                   colo_cst / avcolo * standard_price, rate))

    return dict(bids)


def MedianRate(location):
    rates = [r for _,_,r in Scenario['client_locations'][location]['cluster_scores'] if r > 0]
    m = np.median(rates) if len(rates) > 0 else 0
    return m


# Figures out which bids to accept
def Optimize(bids):
    if len(bids) == 0:
        return {}

    logging.debug('starting optimization')

    m = gurobipy.Model("broker")

    client_locations = Scenario['client_locations'].keys()
    client_locations_map = {location: id for (id, location)
                            in enumerate(client_locations)}
    clients_at_location = defaultdict(int)
    avg_bitrate = defaultdict(list)
    for r in Scenario['requests']:
        id = client_locations_map[r['mgID']]
        clients_at_location[id] += 1
        avg_bitrate[id].append(r['bitrate'])
    for id in avg_bitrate:
        avg_bitrate[id] = sum(avg_bitrate[id]) / float(len(avg_bitrate[id]))

    avg_bitrate = dict(avg_bitrate)

    ################################
    # create variables and objective
    ################################
    logging.debug('starting variables')
    index = defaultdict(dict)
    coeffs = []

    k = 0
    for id, location in enumerate(client_locations):
        if location in bids:

            median_rate = MedianRate(location)

            for j, (cdn, cluster, ping_score, capacity, bw_cst, colo_cst, bw_score) \
                    in enumerate(bids[location]):
                index[id][j] = k
                k += 1

                price = bw_cst + colo_cst
                if not METHOD_SETTINGS[METHOD][1]:  # don't use cost
                    if len(Scenario['CDN_standard_price']) > cdn:
                        price = Scenario['CDN_standard_price'][cdn]
                    else:
                        price = 0
                try:

                    print w_ping, w_bw
                    # if client and cdn haven't talked yet use the median rate of this client to all other cdns
                    if bw_score == 0:
                        bw_score = median_rate * 1.1
                    print 'bw', bw_score, 'ping', ping_score, 'price', price, 'avg rate', avg_bitrate[id]

                    obj_coeff = w_perf * (((w_ping * ping_score) + (w_bw * bw_score))) - (w_cost * (price * avg_bitrate[id]))
                    print 'coeff=', obj_coeff
                except Exception:
                    # we don't have any outstanding requests for this client id
                    # default its avg bitrate to 0
		    logging.debug('no outstanding requests for %s', location)
                    obj_coeff = 0.0
                    avg_bitrate[id] = 0.0
                coeffs.append(obj_coeff)

    # bid "used"
    logging.debug('adding variables')
    U = m.addVars(len(coeffs), lb=0, vtype=gurobipy.GRB.INTEGER, obj=coeffs)

    m.modelSense = gurobipy.GRB.MAXIMIZE
    logging.debug('updating model')
    m.update()

    #############
    # Constraints
    #############
    logging.debug('starting constraints')
    Capacities = {}
    U_cluster = defaultdict(list)
    Bitrates_cluster = defaultdict(list)
    cdn_clusters = set()
    for i, j in [(i, j) for i in index for j in index[i]]:
        location = client_locations[i]
        cdn, cluster, ping, capacity, _, _, rate = bids[location][j]
        cdn_clusters.add((cdn, cluster))

        print 'ping =', ping

        # try to solve problem where a node goes down
        # just make capacity 0 to make the node unappealing
        if ping == 0 or ping == 1000000:
            Capacities[(cdn, cluster)] = 0
        else:
            Capacities[(cdn, cluster)] = capacity
        U_cluster[(cdn, cluster)].append(U[index[i][j]])
        Bitrates_cluster[(cdn, cluster)].append(avg_bitrate[i])

    # don't exceede cluster capacity
    if METHOD_SETTINGS[METHOD][2]:
        logging.debug('adding constraint 1 (proper capacities)')
        m.addConstrs(gurobipy.LinExpr(Bitrates_cluster[c], U_cluster[c]) <=
                     Capacities[c] for c in cdn_clusters)
    else:
        logging.debug('adding constraint 1 (median capacities)')
        m.addConstrs(gurobipy.LinExpr(Bitrates_cluster[c], U_cluster[c]) <=
                     Scenario['median_capacity'][c[0]] for c in cdn_clusters)

    # Make sure all clients are served
    logging.debug('adding constraint 2')
    m.addConstrs(gurobipy.quicksum([U[index[i][j]] for j in index[i]]) ==
                 clients_at_location[i] for i in index)

    ##########
    # Optimize
    ##########
    logging.debug('optimizing')
    m.optimize()

    # see what's happening in gurobi when optimizing
    #m.write("cdn.lp")
    #m.write("cdn.mps")

    #############
    # Get Results
    #############
    logging.debug('getting results')

    try:
        results = {(i, j): round(U[index[i][j]].x)
                   for i in index for j in index[i]}
    except:
        return {}

    for i in index:
        s = sum([results[(i, j)] for j in index[i]])
        if s != clients_at_location[i]:
            logging.debug(i, s, clients_at_location[i])
            logging.debug("ERROR")
#            sys.exit(-1)

    accepted_bids = {}
    for q, r in enumerate(Scenario['requests']):
        id = client_locations_map[r['mgID']]
        bid = None
        for j in index[id]:
            if results[(id, j)] > 0:
                bid = bids[r['mgID']][j]
                results[(id, j)] -= 1
                break
        if bid:
            # FIXME: is this OK? was using q before to map to a particular request
            accepted_bids[r['mgID']] = bid
        else:
            logging.debug("client didn't get a bid")
#            logging.debug(i, j)
#            sys.exit(-1)

    for i, j in [(i, j) for i in index for j in index[i]]:
        if results[(i, j)]:
            logging.debug('ACCEPTED TWO BIDS FOR SAME CLIENT?')
            logging.debug(i, j)
#            sys.exit(-1)

    Scenario['accepted_bids'] = accepted_bids
    return accepted_bids


def GetBids():
    logging.debug('starting cdn bids')
    p = Pool(cpu_count())
    cdns_bids = p.map(GetCDNBids, xrange(len(Scenario['CDNs'])))
    p.close()

    logging.debug('combining cdn bids')
    bids = defaultdict(list)
    for cdn_bids in cdns_bids:
        if cdn_bids:
            for location, bs in cdn_bids.items():
                bids[location] += bs
    return dict(bids)


def store_data(fn, bids, accepted_bids):
    Scenario['CDNs'] = [list(c) for c in Scenario['CDNs']]

    Data = {}
    Data['Scenario'] = Scenario
    Data['Bids'] = bids
    Data['Accepted_Bids'] = accepted_bids

    open(fn, 'w').write(msgpack.packb(Data))

    Scenario['CDNs'] = [set(c) for c in Scenario['CDNs']]
